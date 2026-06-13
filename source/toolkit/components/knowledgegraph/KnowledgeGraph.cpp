/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "KnowledgeGraph.h"
#include "nsNetUtil.h"
#include "nsIDirectoryService.h"
#include "nsAppDirectoryServiceDefs.h"
#include "nsServiceManagerUtils.h"
#include "mozIStorageService.h"
#include "mozIStorageStatement.h"
#include "nsPrintfCString.h"

namespace mozilla {
namespace knowledgegraph {

NS_IMPL_ISUPPORTS(KnowledgeGraph, nsIKnowledgeGraphService)

KnowledgeGraph::KnowledgeGraph() : mInitialized(false) {}

KnowledgeGraph::~KnowledgeGraph() {
  if (mConnection) {
    (void) mConnection->Close();
  }
}

nsresult KnowledgeGraph::GetDatabasePath(nsIFile** aPath) {
  nsCOMPtr<nsIFile> profileDir;
  nsresult rv = NS_GetSpecialDirectory(NS_APP_USER_PROFILE_50_DIR,
                                       getter_AddRefs(profileDir));
  NS_ENSURE_SUCCESS(rv, rv);

  rv = profileDir->AppendNative("knowledgegraph.sqlite"_ns);
  NS_ENSURE_SUCCESS(rv, rv);

  profileDir.forget(aPath);
  return NS_OK;
}

nsresult KnowledgeGraph::EnsureDatabase() {
  if (mInitialized) {
    return NS_OK;
  }

  nsCOMPtr<nsIFile> dbFile;
  nsresult rv = GetDatabasePath(getter_AddRefs(dbFile));
  NS_ENSURE_SUCCESS(rv, rv);

  nsCOMPtr<mozIStorageService> storage =
      do_GetService("@mozilla.org/storage/service;1", &rv);
  NS_ENSURE_SUCCESS(rv, rv);

  rv = storage->OpenDatabase(dbFile, mozIStorageService::CONNECTION_DEFAULT,
                             getter_AddRefs(mConnection));
  NS_ENSURE_SUCCESS(rv, rv);

  // Enable WAL mode for better concurrent performance
  nsCOMPtr<mozIStorageStatement> walStmt;
  rv = mConnection->CreateStatement("PRAGMA journal_mode=WAL"_ns,
                                    getter_AddRefs(walStmt));
  NS_ENSURE_SUCCESS(rv, rv);
  (void) walStmt->Execute();

  rv = CreateSchema();
  NS_ENSURE_SUCCESS(rv, rv);

  mInitialized = true;
  return NS_OK;
}

nsresult KnowledgeGraph::CreateSchema() {
  nsresult rv;

  rv = mConnection->ExecuteSimpleSQL(
      "CREATE TABLE IF NOT EXISTS moz_pages ("
      "  url TEXT PRIMARY KEY,"
      "  title TEXT NOT NULL DEFAULT '',"
      "  text_content TEXT NOT NULL DEFAULT '',"
      "  domain TEXT NOT NULL DEFAULT '',"
      "  visited_at TEXT NOT NULL DEFAULT '',"
      "  indexed_at INTEGER NOT NULL DEFAULT (strftime('%s','now'))"
      ")"_ns);
  NS_ENSURE_SUCCESS(rv, rv);

  // FTS5 is optional — search won't work without it, but basic ops continue
  (void) mConnection->ExecuteSimpleSQL(
      "CREATE VIRTUAL TABLE IF NOT EXISTS moz_pages_fts USING fts5("
      "  title, text_content, content=moz_pages, content_rowid=rowid"
      ")"_ns);

  rv = mConnection->ExecuteSimpleSQL(
      "CREATE TABLE IF NOT EXISTS moz_links ("
      "  source_url TEXT NOT NULL,"
      "  target_url TEXT NOT NULL,"
      "  created_at TEXT NOT NULL,"
      "  PRIMARY KEY (source_url, target_url)"
      ")"_ns);
  NS_ENSURE_SUCCESS(rv, rv);

  rv = mConnection->ExecuteSimpleSQL(
      "CREATE INDEX IF NOT EXISTS idx_pages_domain ON moz_pages(domain)"_ns);
  NS_ENSURE_SUCCESS(rv, rv);

  rv = mConnection->ExecuteSimpleSQL(
      "CREATE INDEX IF NOT EXISTS idx_pages_visited ON moz_pages(visited_at)"_ns);
  NS_ENSURE_SUCCESS(rv, rv);

  // FTS triggers (optional — safe to skip if FTS5 unavailable)
  (void) mConnection->ExecuteSimpleSQL(
      "CREATE TRIGGER IF NOT EXISTS moz_pages_ai AFTER INSERT ON moz_pages BEGIN "
      "  INSERT INTO moz_pages_fts(rowid, title, text_content) "
      "  VALUES (new.rowid, new.title, new.text_content); "
      "END;"_ns);

  (void) mConnection->ExecuteSimpleSQL(
      "CREATE TRIGGER IF NOT EXISTS moz_pages_ad AFTER DELETE ON moz_pages BEGIN "
      "  INSERT INTO moz_pages_fts(moz_pages_fts, rowid, title, text_content) "
      "  VALUES ('delete', old.rowid, old.title, old.text_content); "
      "END;"_ns);

  (void) mConnection->ExecuteSimpleSQL(
      "CREATE TRIGGER IF NOT EXISTS moz_pages_au AFTER UPDATE ON moz_pages BEGIN "
      "  INSERT INTO moz_pages_fts(moz_pages_fts, rowid, title, text_content) "
      "  VALUES ('delete', old.rowid, old.title, old.text_content); "
      "  INSERT INTO moz_pages_fts(rowid, title, text_content) "
      "  VALUES (new.rowid, new.title, new.text_content); "
      "END;"_ns);

  return NS_OK;
}

NS_IMETHODIMP
KnowledgeGraph::IndexPage(const nsACString& aUrl, const nsACString& aTitle,
                          const nsACString& aTextContent,
                          const nsACString& aVisitedAt) {
  nsresult rv = EnsureDatabase();
  NS_ENSURE_SUCCESS(rv, rv);

  // Extract domain from URL
  nsCOMPtr<nsIURI> uri;
  rv = NS_NewURI(getter_AddRefs(uri), aUrl);
  NS_ENSURE_SUCCESS(rv, rv);

  nsAutoCString domain;
  rv = uri->GetHost(domain);
  if (NS_FAILED(rv)) {
    domain = "";
  }

  nsCOMPtr<mozIStorageStatement> stmt;
  rv = mConnection->CreateStatement(
      "INSERT OR REPLACE INTO moz_pages (url, title, text_content, domain, visited_at) "
      "VALUES (?1, ?2, ?3, ?4, ?5)"_ns,
      getter_AddRefs(stmt));
  NS_ENSURE_SUCCESS(rv, rv);

  rv = stmt->BindUTF8StringByIndex(0, aUrl);
  NS_ENSURE_SUCCESS(rv, rv);
  rv = stmt->BindUTF8StringByIndex(1, aTitle);
  NS_ENSURE_SUCCESS(rv, rv);
  rv = stmt->BindUTF8StringByIndex(2, aTextContent);
  NS_ENSURE_SUCCESS(rv, rv);
  rv = stmt->BindUTF8StringByIndex(3, domain);
  NS_ENSURE_SUCCESS(rv, rv);
  rv = stmt->BindUTF8StringByIndex(4, aVisitedAt);
  NS_ENSURE_SUCCESS(rv, rv);

  rv = stmt->Execute();
  NS_ENSURE_SUCCESS(rv, rv);

  return NS_OK;
}

NS_IMETHODIMP
KnowledgeGraph::Search(const nsACString& aQuery, uint32_t aMaxResults,
                       nsACString& aResult) {
  nsresult rv = EnsureDatabase();
  NS_ENSURE_SUCCESS(rv, rv);

  nsCOMPtr<mozIStorageStatement> stmt;
  rv = mConnection->CreateStatement(
      "SELECT p.url, p.title, p.text_content, p.visited_at, p.domain "
      "FROM moz_pages_fts f "
      "JOIN moz_pages p ON p.rowid = f.rowid "
      "WHERE moz_pages_fts MATCH ?1 "
      "ORDER BY rank "
      "LIMIT ?2"_ns,
      getter_AddRefs(stmt));
  NS_ENSURE_SUCCESS(rv, rv);

  rv = stmt->BindUTF8StringByIndex(0, aQuery);
  NS_ENSURE_SUCCESS(rv, rv);
  rv = stmt->BindInt32ByIndex(1, aMaxResults);
  NS_ENSURE_SUCCESS(rv, rv);

  nsAutoCString json("[");
  bool first = true;

  bool hasMore;
  while (NS_SUCCEEDED(stmt->ExecuteStep(&hasMore)) && hasMore) {
    if (!first) {
      json.Append(",");
    }
    first = false;

    nsAutoCString url, title, textContent, visitedAt, domain;
    (void) stmt->GetUTF8String(0, url);
    (void) stmt->GetUTF8String(1, title);
    (void) stmt->GetUTF8String(2, textContent);
    (void) stmt->GetUTF8String(3, visitedAt);
    (void) stmt->GetUTF8String(4, domain);

    // Escape JSON strings
    nsAutoCString escapedTitle(title);
    escapedTitle.ReplaceSubstring("\\", "\\\\");
    escapedTitle.ReplaceSubstring("\"", "\\\"");
    escapedTitle.ReplaceSubstring("\n", "\\n");
    escapedTitle.ReplaceSubstring("\r", "\\r");
    escapedTitle.ReplaceSubstring("\t", "\\t");

    nsAutoCString escapedText(textContent);
    escapedText.ReplaceSubstring("\\", "\\\\");
    escapedText.ReplaceSubstring("\"", "\\\"");
    escapedText.ReplaceSubstring("\n", "\\n");
    escapedText.ReplaceSubstring("\r", "\\r");
    escapedText.ReplaceSubstring("\t", "\\t");

    json.Append(nsPrintfCString(
        "{\"url\":\"%s\",\"title\":\"%s\",\"text_content\":\"%s\","
        "\"visited_at\":\"%s\",\"domain\":\"%s\"}",
        url.get(), escapedTitle.get(), escapedText.get(),
        visitedAt.get(), domain.get()));
  }

  json.Append("]");
  aResult = json;
  return NS_OK;
}

NS_IMETHODIMP
KnowledgeGraph::GetRelated(const nsACString& aUrl, uint32_t aMaxResults,
                           nsACString& aResult) {
  nsresult rv = EnsureDatabase();
  NS_ENSURE_SUCCESS(rv, rv);

  // Extract domain from URL for related content
  nsCOMPtr<nsIURI> uri;
  rv = NS_NewURI(getter_AddRefs(uri), aUrl);
  NS_ENSURE_SUCCESS(rv, rv);

  nsAutoCString domain;
  rv = uri->GetHost(domain);
  NS_ENSURE_SUCCESS(rv, rv);

  nsCOMPtr<mozIStorageStatement> stmt;
  rv = mConnection->CreateStatement(
      "SELECT url, title, text_content, visited_at, domain "
      "FROM moz_pages "
      "WHERE domain = ?1 AND url != ?2 "
      "ORDER BY indexed_at DESC "
      "LIMIT ?3"_ns,
      getter_AddRefs(stmt));
  NS_ENSURE_SUCCESS(rv, rv);

  rv = stmt->BindUTF8StringByIndex(0, domain);
  NS_ENSURE_SUCCESS(rv, rv);
  rv = stmt->BindUTF8StringByIndex(1, aUrl);
  NS_ENSURE_SUCCESS(rv, rv);
  rv = stmt->BindInt32ByIndex(2, aMaxResults);
  NS_ENSURE_SUCCESS(rv, rv);

  nsAutoCString json("[");
  bool first = true;

  bool hasMore;
  while (NS_SUCCEEDED(stmt->ExecuteStep(&hasMore)) && hasMore) {
    if (!first) {
      json.Append(",");
    }
    first = false;

    nsAutoCString url, title, textContent, visitedAt, pageDomain;
    (void) stmt->GetUTF8String(0, url);
    (void) stmt->GetUTF8String(1, title);
    (void) stmt->GetUTF8String(2, textContent);
    (void) stmt->GetUTF8String(3, visitedAt);
    (void) stmt->GetUTF8String(4, pageDomain);

    nsAutoCString escapedTitle(title);
    escapedTitle.ReplaceSubstring("\\", "\\\\");
    escapedTitle.ReplaceSubstring("\"", "\\\"");
    escapedTitle.ReplaceSubstring("\n", "\\n");
    escapedTitle.ReplaceSubstring("\r", "\\r");
    escapedTitle.ReplaceSubstring("\t", "\\t");

    json.Append(nsPrintfCString(
        "{\"url\":\"%s\",\"title\":\"%s\",\"visited_at\":\"%s\",\"domain\":\"%s\"}",
        url.get(), escapedTitle.get(), visitedAt.get(), pageDomain.get()));
  }

  json.Append("]");
  aResult = json;
  return NS_OK;
}

NS_IMETHODIMP
KnowledgeGraph::GetRecent(uint32_t aMaxResults, nsACString& aResult) {
  nsresult rv = EnsureDatabase();
  NS_ENSURE_SUCCESS(rv, rv);

  nsCOMPtr<mozIStorageStatement> stmt;
  rv = mConnection->CreateStatement(
      "SELECT url, title, text_content, visited_at, domain "
      "FROM moz_pages "
      "ORDER BY visited_at DESC "
      "LIMIT ?1"_ns,
      getter_AddRefs(stmt));
  NS_ENSURE_SUCCESS(rv, rv);

  rv = stmt->BindInt32ByIndex(0, aMaxResults);
  NS_ENSURE_SUCCESS(rv, rv);

  nsAutoCString json("[");
  bool first = true;

  bool hasMore;
  while (NS_SUCCEEDED(stmt->ExecuteStep(&hasMore)) && hasMore) {
    if (!first) {
      json.Append(",");
    }
    first = false;

    nsAutoCString url, title, textContent, visitedAt, domain;
    (void) stmt->GetUTF8String(0, url);
    (void) stmt->GetUTF8String(1, title);
    (void) stmt->GetUTF8String(2, textContent);
    (void) stmt->GetUTF8String(3, visitedAt);
    (void) stmt->GetUTF8String(4, domain);

    nsAutoCString escapedTitle(title);
    escapedTitle.ReplaceSubstring("\\", "\\\\");
    escapedTitle.ReplaceSubstring("\"", "\\\"");
    escapedTitle.ReplaceSubstring("\n", "\\n");
    escapedTitle.ReplaceSubstring("\r", "\\r");
    escapedTitle.ReplaceSubstring("\t", "\\t");

    json.Append(nsPrintfCString(
        "{\"url\":\"%s\",\"title\":\"%s\",\"visited_at\":\"%s\",\"domain\":\"%s\"}",
        url.get(), escapedTitle.get(), visitedAt.get(), domain.get()));
  }

  json.Append("]");
  aResult = json;
  return NS_OK;
}

NS_IMETHODIMP
KnowledgeGraph::GetPageCount(uint32_t* aCount) {
  nsresult rv = EnsureDatabase();
  NS_ENSURE_SUCCESS(rv, rv);

  nsCOMPtr<mozIStorageStatement> stmt;
  rv = mConnection->CreateStatement(
      "SELECT COUNT(*) FROM moz_pages"_ns, getter_AddRefs(stmt));
  NS_ENSURE_SUCCESS(rv, rv);

  bool hasMore;
  rv = stmt->ExecuteStep(&hasMore);
  NS_ENSURE_SUCCESS(rv, rv);

  if (hasMore) {
    rv = stmt->GetInt32(0, reinterpret_cast<int32_t*>(aCount));
    NS_ENSURE_SUCCESS(rv, rv);
  } else {
    *aCount = 0;
  }

  return NS_OK;
}

NS_IMETHODIMP
KnowledgeGraph::Clear() {
  nsresult rv = EnsureDatabase();
  NS_ENSURE_SUCCESS(rv, rv);

  rv = mConnection->ExecuteSimpleSQL("DELETE FROM moz_pages"_ns);
  NS_ENSURE_SUCCESS(rv, rv);

  rv = mConnection->ExecuteSimpleSQL("DELETE FROM moz_links"_ns);
  NS_ENSURE_SUCCESS(rv, rv);

  rv = mConnection->ExecuteSimpleSQL("INSERT INTO moz_pages_fts(moz_pages_fts) VALUES('rebuild')"_ns);
  NS_ENSURE_SUCCESS(rv, rv);

  return NS_OK;
}

}  // namespace knowledgegraph
}  // namespace mozilla
