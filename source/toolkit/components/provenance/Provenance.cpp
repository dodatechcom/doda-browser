#include "Provenance.h"
#include "nsNetUtil.h"
#include "nsIDirectoryService.h"
#include "nsAppDirectoryServiceDefs.h"
#include "nsServiceManagerUtils.h"
#include "mozIStorageService.h"
#include "mozIStorageStatement.h"
#include "nsPrintfCString.h"
#include "nsIInputStream.h"
#include "nsIChannel.h"
#include "nsIHttpChannel.h"
#include "nsIUploadChannel2.h"
#include "nsStringStream.h"
#include "nsContentUtils.h"
#include "mozilla/HashFunctions.h"
#include "mozilla/Components.h"

namespace mozilla {
namespace provenance {

NS_IMPL_ISUPPORTS(Provenance, nsIProvenanceService)

Provenance::Provenance() : mInitialized(false) {}

Provenance::~Provenance() {
  if (mConnection) {
    (void) mConnection->Close();
  }
}

nsresult Provenance::GetDatabasePath(nsIFile** aPath) {
  nsCOMPtr<nsIFile> profileDir;
  nsresult rv = NS_GetSpecialDirectory(NS_APP_USER_PROFILE_50_DIR,
                                       getter_AddRefs(profileDir));
  NS_ENSURE_SUCCESS(rv, rv);

  rv = profileDir->AppendNative("provenance.sqlite"_ns);
  NS_ENSURE_SUCCESS(rv, rv);

  profileDir.forget(aPath);
  return NS_OK;
}

nsresult Provenance::EnsureDatabase() {
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

  rv = CreateSchema();
  NS_ENSURE_SUCCESS(rv, rv);

  mInitialized = true;
  return NS_OK;
}

nsresult Provenance::CreateSchema() {
  nsresult rv;

  rv = mConnection->ExecuteSimpleSQL(
      "CREATE TABLE IF NOT EXISTS moz_claims ("
      "  id TEXT PRIMARY KEY,"
      "  claim_text TEXT NOT NULL,"
      "  source_url TEXT NOT NULL,"
      "  source_title TEXT NOT NULL DEFAULT '',"
      "  domain TEXT NOT NULL DEFAULT '',"
      "  created_at INTEGER NOT NULL DEFAULT (strftime('%s','now')),"
      "  confidence REAL NOT NULL DEFAULT 0.5"
      ")"_ns);
  NS_ENSURE_SUCCESS(rv, rv);

  rv = mConnection->ExecuteSimpleSQL(
      "CREATE INDEX IF NOT EXISTS idx_claims_domain ON moz_claims(domain)"_ns);
  NS_ENSURE_SUCCESS(rv, rv);

  rv = mConnection->ExecuteSimpleSQL(
      "CREATE INDEX IF NOT EXISTS idx_claims_created ON moz_claims(created_at)"_ns);
  NS_ENSURE_SUCCESS(rv, rv);

  return NS_OK;
}

nsresult Provenance::OllamaRequest(const nsACString& aEndpoint,
                                   const nsACString& aBody,
                                   nsACString& aResult) {
  nsresult rv;

  nsAutoCString url("http://127.0.0.1:11434");
  url.Append(aEndpoint);

  nsCOMPtr<nsIURI> uri;
  rv = NS_NewURI(getter_AddRefs(uri), url);
  NS_ENSURE_SUCCESS(rv, rv);

  nsCOMPtr<nsIChannel> channel;
  rv = NS_NewChannel(getter_AddRefs(channel), uri,
                     nsContentUtils::GetSystemPrincipal(),
                     nsILoadInfo::SEC_ALLOW_CROSS_ORIGIN_SEC_CONTEXT_IS_NULL,
                     nsIContentPolicy::TYPE_OTHER);
  NS_ENSURE_SUCCESS(rv, rv);

  nsCOMPtr<nsIHttpChannel> httpChannel = do_QueryInterface(channel, &rv);
  NS_ENSURE_SUCCESS(rv, rv);

  rv = httpChannel->SetRequestMethod("POST"_ns);
  NS_ENSURE_SUCCESS(rv, rv);

  rv = httpChannel->SetRequestHeader("Content-Type"_ns,
                                     "application/json"_ns, false);
  NS_ENSURE_SUCCESS(rv, rv);

  nsCOMPtr<nsIUploadChannel2> uploadChannel = do_QueryInterface(channel, &rv);
  NS_ENSURE_SUCCESS(rv, rv);

  nsCOMPtr<nsIInputStream> uploadStream;
  rv = NS_NewCStringInputStream(getter_AddRefs(uploadStream), aBody);
  NS_ENSURE_SUCCESS(rv, rv);

  rv = uploadChannel->ExplicitSetUploadStream(
      uploadStream, "application/json"_ns, aBody.Length(), "POST"_ns, false);
  NS_ENSURE_SUCCESS(rv, rv);

  nsCOMPtr<nsIInputStream> responseStream;
  rv = channel->Open(getter_AddRefs(responseStream));
  NS_ENSURE_SUCCESS(rv, rv);

  uint64_t available;
  rv = responseStream->Available(&available);
  NS_ENSURE_SUCCESS(rv, rv);

  if (available > 0) {
    nsAutoCString buffer;
    rv = NS_ReadInputStreamToString(responseStream, buffer, available);
    NS_ENSURE_SUCCESS(rv, rv);
    aResult = buffer;
  }

  return NS_OK;
}

NS_IMETHODIMP
Provenance::ExtractClaims(const nsACString& aUrl, const nsACString& aTitle,
                          const nsACString& aTextContent, nsACString& aResult) {
  nsresult rv = EnsureDatabase();
  NS_ENSURE_SUCCESS(rv, rv);

  // Build Ollama prompt for claim extraction
  nsAutoCString prompt;
  prompt.Append("Extract factual claims from the following text. ");
  prompt.Append("For each claim, provide a JSON object with 'claim' (the factual statement), ");
  prompt.Append("'confidence' (0.0-1.0), and 'category' (one of: health, politics, science, technology, business, sports, entertainment, other). ");
  prompt.Append("Return a JSON array of claim objects only, no other text.\n\nText:\n");
  prompt.Append(aTextContent);

  nsAutoCString body;
  body.Append("{\"model\":\"llama3.2\",\"messages\":[{\"role\":\"user\",\"content\":\"");
  nsAutoCString escaped(prompt);
  escaped.ReplaceSubstring("\\", "\\\\");
  escaped.ReplaceSubstring("\"", "\\\"");
  escaped.ReplaceSubstring("\n", "\\n");
  escaped.ReplaceSubstring("\r", "\\r");
  escaped.ReplaceSubstring("\t", "\\t");
  body.Append(escaped);
  body.Append("\"}],\"stream\":false}");

  nsAutoCString response;
  rv = OllamaRequest("/api/chat"_ns, body, response);
  NS_ENSURE_SUCCESS(rv, rv);

  // Store extracted claims
  rv = ParseClaimsFromResponse(response, aUrl, aTitle);
  NS_ENSURE_SUCCESS(rv, rv);

  aResult = response;
  return NS_OK;
}

nsresult Provenance::ParseClaimsFromResponse(const nsACString& aResponse,
                                              const nsACString& aUrl,
                                              const nsACString& aTitle) {
  // Extract domain from URL
  nsCOMPtr<nsIURI> uri;
  nsresult rv = NS_NewURI(getter_AddRefs(uri), aUrl);
  NS_ENSURE_SUCCESS(rv, rv);

  nsAutoCString domain;
  rv = uri->GetHost(domain);
  if (NS_FAILED(rv)) {
    domain = ""_ns;
  }

  // Simple JSON parsing — find claim objects and store them
  // We look for "claim":"..." patterns in the response
  int32_t pos = 0;
  nsAutoCString remaining(aResponse);
  nsAutoCString claimText;

  while (true) {
    int32_t claimStart = remaining.Find("\"claim\":\"");
    if (claimStart == -1) {
      break;
    }
    claimStart += 10; // Skip past "\"claim\":\""
    int32_t claimEnd = remaining.Find("\"", claimStart);
    if (claimEnd == -1) {
      break;
    }

    claimText = Substring(remaining, claimStart, claimEnd - claimStart);
    rv = StoreClaim(claimText, aUrl, aTitle, domain);
    if (NS_FAILED(rv)) {
      // Continue extracting other claims even if one fails
      NS_WARNING("Provenance: failed to store claim");
    }

    remaining.Cut(0, claimEnd + 1);
    pos += claimEnd + 1;
    if (pos > (int32_t)aResponse.Length()) {
      break;
    }
  }

  return NS_OK;
}

nsresult Provenance::StoreClaim(const nsACString& aText, const nsACString& aUrl,
                                 const nsACString& aTitle,
                                 const nsACString& aDomain) {
  // Generate a hash-based ID from URL + text
  nsAutoCString idInput(aUrl);
  idInput.Append(":");
  idInput.Append(aText);

  uint32_t hash = HashString(idInput);
  nsAutoCString id(nsPrintfCString("claim_%08x", hash));

  nsCOMPtr<mozIStorageStatement> stmt;
  nsresult rv = mConnection->CreateStatement(
      "INSERT OR IGNORE INTO moz_claims (id, claim_text, source_url, source_title, domain) "
      "VALUES (?1, ?2, ?3, ?4, ?5)"_ns,
      getter_AddRefs(stmt));
  NS_ENSURE_SUCCESS(rv, rv);

  rv = stmt->BindUTF8StringByIndex(0, id);
  NS_ENSURE_SUCCESS(rv, rv);
  rv = stmt->BindUTF8StringByIndex(1, aText);
  NS_ENSURE_SUCCESS(rv, rv);
  rv = stmt->BindUTF8StringByIndex(2, aUrl);
  NS_ENSURE_SUCCESS(rv, rv);
  rv = stmt->BindUTF8StringByIndex(3, aTitle);
  NS_ENSURE_SUCCESS(rv, rv);
  rv = stmt->BindUTF8StringByIndex(4, aDomain);
  NS_ENSURE_SUCCESS(rv, rv);

  rv = stmt->Execute();
  NS_ENSURE_SUCCESS(rv, rv);

  return NS_OK;
}

NS_IMETHODIMP
Provenance::CompareSources(const nsACString& aClaimId, nsACString& aResult) {
  nsresult rv = EnsureDatabase();
  NS_ENSURE_SUCCESS(rv, rv);

  // Find all sources for a similar claim (by claim ID prefix matching)
  nsCOMPtr<mozIStorageStatement> stmt;
  rv = mConnection->CreateStatement(
      "SELECT claim_text, source_url, source_title, domain, created_at, confidence "
      "FROM moz_claims WHERE id = ?1"_ns,
      getter_AddRefs(stmt));
  NS_ENSURE_SUCCESS(rv, rv);

  rv = stmt->BindUTF8StringByIndex(0, aClaimId);
  NS_ENSURE_SUCCESS(rv, rv);

  nsAutoCString json("[");
  bool first = true;

  bool hasMore;
  while (NS_SUCCEEDED(stmt->ExecuteStep(&hasMore)) && hasMore) {
    if (!first) {
      json.Append(",");
    }
    first = false;

    nsAutoCString claimText, url, title, domain;
    int64_t createdAt;
    double confidence;
    (void) stmt->GetUTF8String(0, claimText);
    (void) stmt->GetUTF8String(1, url);
    (void) stmt->GetUTF8String(2, title);
    (void) stmt->GetUTF8String(3, domain);
    (void) stmt->GetInt64(4, &createdAt);
    (void) stmt->GetDouble(5, &confidence);

    nsAutoCString escapedClaim(claimText);
    escapedClaim.ReplaceSubstring("\\", "\\\\");
    escapedClaim.ReplaceSubstring("\"", "\\\"");
    escapedClaim.ReplaceSubstring("\n", "\\n");

    json.Append(nsPrintfCString(
        "{\"claim_text\":\"%s\",\"source_url\":\"%s\",\"source_title\":\"%s\","
        "\"domain\":\"%s\",\"created_at\":%lld,\"confidence\":%f}",
        escapedClaim.get(), url.get(), title.get(),
        domain.get(), static_cast<long long>(createdAt), confidence));
  }

  json.Append("]");
  aResult = json;
  return NS_OK;
}

NS_IMETHODIMP
Provenance::FindSpread(const nsACString& aClaimId, uint32_t aMaxResults,
                        nsACString& aResult) {
  nsresult rv = EnsureDatabase();
  NS_ENSURE_SUCCESS(rv, rv);

  // Get the claim text first
  nsCOMPtr<mozIStorageStatement> getStmt;
  rv = mConnection->CreateStatement(
      "SELECT claim_text FROM moz_claims WHERE id = ?1"_ns,
      getter_AddRefs(getStmt));
  NS_ENSURE_SUCCESS(rv, rv);

  rv = getStmt->BindUTF8StringByIndex(0, aClaimId);
  NS_ENSURE_SUCCESS(rv, rv);

  bool hasMore;
  rv = getStmt->ExecuteStep(&hasMore);
  NS_ENSURE_SUCCESS(rv, rv);

  nsAutoCString claimText;
  if (hasMore) {
    (void) getStmt->GetUTF8String(0, claimText);
  } else {
    aResult = "[]"_ns;
    return NS_OK;
  }

  // Find similar claims across other sources
  nsCOMPtr<mozIStorageStatement> stmt;
  rv = mConnection->CreateStatement(
      "SELECT id, claim_text, source_url, source_title, domain, created_at "
      "FROM moz_claims WHERE claim_text = ?1 AND id != ?2 "
      "ORDER BY created_at DESC LIMIT ?3"_ns,
      getter_AddRefs(stmt));
  NS_ENSURE_SUCCESS(rv, rv);

  rv = stmt->BindUTF8StringByIndex(0, claimText);
  NS_ENSURE_SUCCESS(rv, rv);
  rv = stmt->BindUTF8StringByIndex(1, aClaimId);
  NS_ENSURE_SUCCESS(rv, rv);
  rv = stmt->BindInt32ByIndex(2, aMaxResults);
  NS_ENSURE_SUCCESS(rv, rv);

  nsAutoCString json("[");
  bool first = true;

  while (NS_SUCCEEDED(stmt->ExecuteStep(&hasMore)) && hasMore) {
    if (!first) {
      json.Append(",");
    }
    first = false;

    nsAutoCString id, text, url, title, domain;
    int64_t createdAt;
    (void) stmt->GetUTF8String(0, id);
    (void) stmt->GetUTF8String(1, text);
    (void) stmt->GetUTF8String(2, url);
    (void) stmt->GetUTF8String(3, title);
    (void) stmt->GetUTF8String(4, domain);
    (void) stmt->GetInt64(5, &createdAt);

    json.Append(nsPrintfCString(
        "{\"id\":\"%s\",\"claim_text\":\"%s\",\"source_url\":\"%s\","
        "\"source_title\":\"%s\",\"domain\":\"%s\",\"created_at\":%lld}",
        id.get(), text.get(), url.get(), title.get(),
        domain.get(), static_cast<long long>(createdAt)));
  }

  json.Append("]");
  aResult = json;
  return NS_OK;
}

NS_IMETHODIMP
Provenance::SearchClaims(const nsACString& aQuery, uint32_t aMaxResults,
                          nsACString& aResult) {
  nsresult rv = EnsureDatabase();
  NS_ENSURE_SUCCESS(rv, rv);

  nsCOMPtr<mozIStorageStatement> stmt;
  rv = mConnection->CreateStatement(
      "SELECT id, claim_text, source_url, source_title, domain, created_at, confidence "
      "FROM moz_claims WHERE claim_text LIKE ?1 "
      "ORDER BY created_at DESC LIMIT ?2"_ns,
      getter_AddRefs(stmt));
  NS_ENSURE_SUCCESS(rv, rv);

  nsAutoCString likePattern("%");
  likePattern.Append(aQuery);
  likePattern.Append("%");
  rv = stmt->BindUTF8StringByIndex(0, likePattern);
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

    nsAutoCString id, claimText, url, title, domain;
    int64_t createdAt;
    double confidence;
    (void) stmt->GetUTF8String(0, id);
    (void) stmt->GetUTF8String(1, claimText);
    (void) stmt->GetUTF8String(2, url);
    (void) stmt->GetUTF8String(3, title);
    (void) stmt->GetUTF8String(4, domain);
    (void) stmt->GetInt64(5, &createdAt);
    (void) stmt->GetDouble(6, &confidence);

    nsAutoCString escapedClaim(claimText);
    escapedClaim.ReplaceSubstring("\\", "\\\\");
    escapedClaim.ReplaceSubstring("\"", "\\\"");
    escapedClaim.ReplaceSubstring("\n", "\\n");

    json.Append(nsPrintfCString(
        "{\"id\":\"%s\",\"claim_text\":\"%s\",\"source_url\":\"%s\","
        "\"source_title\":\"%s\",\"domain\":\"%s\",\"created_at\":%lld,"
        "\"confidence\":%f}",
        id.get(), escapedClaim.get(), url.get(), title.get(),
        domain.get(), static_cast<long long>(createdAt), confidence));
  }

  json.Append("]");
  aResult = json;
  return NS_OK;
}

NS_IMETHODIMP
Provenance::GetRecent(uint32_t aMaxResults, nsACString& aResult) {
  nsresult rv = EnsureDatabase();
  NS_ENSURE_SUCCESS(rv, rv);

  nsCOMPtr<mozIStorageStatement> stmt;
  rv = mConnection->CreateStatement(
      "SELECT id, claim_text, source_url, source_title, domain, created_at, confidence "
      "FROM moz_claims ORDER BY created_at DESC LIMIT ?1"_ns,
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

    nsAutoCString id, claimText, url, title, domain;
    int64_t createdAt;
    double confidence;
    (void) stmt->GetUTF8String(0, id);
    (void) stmt->GetUTF8String(1, claimText);
    (void) stmt->GetUTF8String(2, url);
    (void) stmt->GetUTF8String(3, title);
    (void) stmt->GetUTF8String(4, domain);
    (void) stmt->GetInt64(5, &createdAt);
    (void) stmt->GetDouble(6, &confidence);

    nsAutoCString escapedClaim(claimText);
    escapedClaim.ReplaceSubstring("\\", "\\\\");
    escapedClaim.ReplaceSubstring("\"", "\\\"");
    escapedClaim.ReplaceSubstring("\n", "\\n");

    json.Append(nsPrintfCString(
        "{\"id\":\"%s\",\"claim_text\":\"%s\",\"source_url\":\"%s\","
        "\"source_title\":\"%s\",\"domain\":\"%s\",\"created_at\":%lld,"
        "\"confidence\":%f}",
        id.get(), escapedClaim.get(), url.get(), title.get(),
        domain.get(), static_cast<long long>(createdAt), confidence));
  }

  json.Append("]");
  aResult = json;
  return NS_OK;
}

NS_IMETHODIMP
Provenance::GetClaimCount(uint32_t* aCount) {
  nsresult rv = EnsureDatabase();
  NS_ENSURE_SUCCESS(rv, rv);

  nsCOMPtr<mozIStorageStatement> stmt;
  rv = mConnection->CreateStatement(
      "SELECT COUNT(*) FROM moz_claims"_ns, getter_AddRefs(stmt));
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
Provenance::Clear() {
  nsresult rv = EnsureDatabase();
  NS_ENSURE_SUCCESS(rv, rv);

  rv = mConnection->ExecuteSimpleSQL("DELETE FROM moz_claims"_ns);
  NS_ENSURE_SUCCESS(rv, rv);

  return NS_OK;
}

}  // namespace provenance
}  // namespace mozilla
