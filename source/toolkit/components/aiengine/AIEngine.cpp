#include "AIEngine.h"
#include "nsNetUtil.h"
#include "nsIDirectoryService.h"
#include "nsAppDirectoryServiceDefs.h"
#include "nsServiceManagerUtils.h"
#include "mozIStorageService.h"
#include "mozIStorageStatement.h"
#include "nsPrintfCString.h"
#include "nsIInputStream.h"
#include "nsIOutputStream.h"
#include "nsIChannel.h"
#include "nsIHttpChannel.h"
#include "nsIUploadChannel2.h"
#include "nsStringStream.h"
#include "nsContentUtils.h"
#include "mozilla/Components.h"

namespace mozilla {
namespace aiengine {

NS_IMPL_ISUPPORTS(AIEngine, nsIAIEngineService)

AIEngine::AIEngine() : mModel("llama3.2"_ns), mInitialized(false) {}

AIEngine::~AIEngine() {
  if (mConnection) {
    (void) mConnection->Close();
  }
}

nsresult AIEngine::GetDatabasePath(nsIFile** aPath) {
  nsCOMPtr<nsIFile> profileDir;
  nsresult rv = NS_GetSpecialDirectory(NS_APP_USER_PROFILE_50_DIR,
                                       getter_AddRefs(profileDir));
  NS_ENSURE_SUCCESS(rv, rv);

  rv = profileDir->AppendNative("aiengine.sqlite"_ns);
  NS_ENSURE_SUCCESS(rv, rv);

  profileDir.forget(aPath);
  return NS_OK;
}

nsresult AIEngine::EnsureDatabase() {
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

nsresult AIEngine::CreateSchema() {
  nsresult rv;

  rv = mConnection->ExecuteSimpleSQL(
      "CREATE TABLE IF NOT EXISTS moz_history ("
      "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
      "  mode TEXT NOT NULL,"
      "  role TEXT NOT NULL,"
      "  content TEXT NOT NULL,"
      "  created_at INTEGER NOT NULL DEFAULT (strftime('%s','now'))"
      ")"_ns);
  NS_ENSURE_SUCCESS(rv, rv);

  rv = mConnection->ExecuteSimpleSQL(
      "CREATE INDEX IF NOT EXISTS idx_history_created "
      "ON moz_history(created_at)"_ns);
  NS_ENSURE_SUCCESS(rv, rv);

  return NS_OK;
}

nsresult AIEngine::OllamaRequest(const nsACString& aEndpoint,
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
AIEngine::CheckHealth(bool* aResult) {
  *aResult = false;

  nsCOMPtr<nsIURI> uri;
  nsresult rv = NS_NewURI(getter_AddRefs(uri), "http://127.0.0.1:11434"_ns);
  NS_ENSURE_SUCCESS(rv, rv);

  nsCOMPtr<nsIChannel> channel;
  rv = NS_NewChannel(getter_AddRefs(channel), uri,
                     nsContentUtils::GetSystemPrincipal(),
                     nsILoadInfo::SEC_ALLOW_CROSS_ORIGIN_SEC_CONTEXT_IS_NULL,
                     nsIContentPolicy::TYPE_OTHER);
  NS_ENSURE_SUCCESS(rv, rv);

  nsCOMPtr<nsIHttpChannel> httpChannel = do_QueryInterface(channel, &rv);
  NS_ENSURE_SUCCESS(rv, rv);

  rv = httpChannel->SetRequestMethod("HEAD"_ns);
  NS_ENSURE_SUCCESS(rv, rv);

  nsCOMPtr<nsIInputStream> responseStream;
  rv = channel->Open(getter_AddRefs(responseStream));
  if (NS_SUCCEEDED(rv)) {
    *aResult = true;
  }

  return NS_OK;
}

NS_IMETHODIMP
AIEngine::Chat(const nsACString& aMessage, const nsACString& aHistory,
               nsACString& aResult) {
  nsresult rv = EnsureDatabase();
  NS_ENSURE_SUCCESS(rv, rv);

  // Store user message
  nsCOMPtr<mozIStorageStatement> insertStmt;
  rv = mConnection->CreateStatement(
      "INSERT INTO moz_history (mode, role, content) VALUES ('chat', 'user', ?1)"_ns,
      getter_AddRefs(insertStmt));
  NS_ENSURE_SUCCESS(rv, rv);

  rv = insertStmt->BindUTF8StringByIndex(0, aMessage);
  NS_ENSURE_SUCCESS(rv, rv);

  rv = insertStmt->Execute();
  NS_ENSURE_SUCCESS(rv, rv);

  // Build Ollama request body
  nsAutoCString body;
  body.Append("{\"model\":\"");
  body.Append(mModel);
  body.Append("\",\"messages\":[");
  body.Append(aHistory);
  if (!aHistory.IsEmpty()) {
    body.Append(",");
  }
  body.Append("{\"role\":\"user\",\"content\":\"");
  // Escape JSON string
  nsAutoCString escaped(aMessage);
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

  // Store assistant response
  nsCOMPtr<mozIStorageStatement> respStmt;
  rv = mConnection->CreateStatement(
      "INSERT INTO moz_history (mode, role, content) VALUES ('chat', 'assistant', ?1)"_ns,
      getter_AddRefs(respStmt));
  NS_ENSURE_SUCCESS(rv, rv);

  rv = respStmt->BindUTF8StringByIndex(0, response);
  NS_ENSURE_SUCCESS(rv, rv);

  (void) respStmt->Execute();

  aResult = response;
  return NS_OK;
}

NS_IMETHODIMP
AIEngine::AnalyzeTab(const nsACString& aPageContent,
                     const nsACString& aPrompt, nsACString& aResult) {
  nsresult rv = EnsureDatabase();
  NS_ENSURE_SUCCESS(rv, rv);

  nsAutoCString content;
  content.Append("Analyze the following page content. ");
  content.Append(aPrompt);
  content.Append("\n\nPage content:\n");
  content.Append(aPageContent);

  nsAutoCString body;
  body.Append("{\"model\":\"");
  body.Append(mModel);
  body.Append("\",\"messages\":[{\"role\":\"user\",\"content\":\"");
  nsAutoCString escaped(content);
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

  aResult = response;
  return NS_OK;
}

NS_IMETHODIMP
AIEngine::CrossPage(const nsACString& aPages, const nsACString& aPrompt,
                    nsACString& aResult) {
  nsresult rv = EnsureDatabase();
  NS_ENSURE_SUCCESS(rv, rv);

  nsAutoCString content;
  content.Append("Compare and analyze the following pages. ");
  content.Append(aPrompt);
  content.Append("\n\nPages:\n");
  content.Append(aPages);

  nsAutoCString body;
  body.Append("{\"model\":\"");
  body.Append(mModel);
  body.Append("\",\"messages\":[{\"role\":\"user\",\"content\":\"");
  nsAutoCString escaped(content);
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

  aResult = response;
  return NS_OK;
}

NS_IMETHODIMP
AIEngine::Debate(const nsACString& aTopic, const nsACString& aPerspectives,
                 nsACString& aResult) {
  nsresult rv = EnsureDatabase();
  NS_ENSURE_SUCCESS(rv, rv);

  nsAutoCString content;
  content.Append("Debate the following topic considering multiple perspectives.\n\n");
  content.Append("Topic: ");
  content.Append(aTopic);
  content.Append("\n\nPerspectives to consider:\n");
  content.Append(aPerspectives);
  content.Append("\n\nPresent arguments from each perspective, then provide a balanced synthesis.");

  nsAutoCString body;
  body.Append("{\"model\":\"");
  body.Append(mModel);
  body.Append("\",\"messages\":[{\"role\":\"user\",\"content\":\"");
  nsAutoCString escaped(content);
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

  aResult = response;
  return NS_OK;
}

NS_IMETHODIMP
AIEngine::ClearHistory() {
  nsresult rv = EnsureDatabase();
  NS_ENSURE_SUCCESS(rv, rv);

  rv = mConnection->ExecuteSimpleSQL("DELETE FROM moz_history"_ns);
  NS_ENSURE_SUCCESS(rv, rv);

  return NS_OK;
}

NS_IMETHODIMP
AIEngine::GetHistory(uint32_t aLimit, nsACString& aResult) {
  nsresult rv = EnsureDatabase();
  NS_ENSURE_SUCCESS(rv, rv);

  nsCOMPtr<mozIStorageStatement> stmt;
  rv = mConnection->CreateStatement(
      "SELECT mode, role, content, created_at FROM moz_history "
      "ORDER BY created_at DESC LIMIT ?1"_ns,
      getter_AddRefs(stmt));
  NS_ENSURE_SUCCESS(rv, rv);

  rv = stmt->BindInt32ByIndex(0, aLimit);
  NS_ENSURE_SUCCESS(rv, rv);

  nsAutoCString json("[");
  bool first = true;

  bool hasMore;
  while (NS_SUCCEEDED(stmt->ExecuteStep(&hasMore)) && hasMore) {
    if (!first) {
      json.Append(",");
    }
    first = false;

    nsAutoCString mode, role, content;
    int64_t createdAt;
    (void) stmt->GetUTF8String(0, mode);
    (void) stmt->GetUTF8String(1, role);
    (void) stmt->GetUTF8String(2, content);
    (void) stmt->GetInt64(3, &createdAt);

    nsAutoCString escapedContent(content);
    escapedContent.ReplaceSubstring("\\", "\\\\");
    escapedContent.ReplaceSubstring("\"", "\\\"");
    escapedContent.ReplaceSubstring("\n", "\\n");
    escapedContent.ReplaceSubstring("\r", "\\r");
    escapedContent.ReplaceSubstring("\t", "\\t");

    json.Append(nsPrintfCString(
        "{\"mode\":\"%s\",\"role\":\"%s\",\"content\":\"%s\",\"created_at\":%lld}",
        mode.get(), role.get(), escapedContent.get(),
        static_cast<long long>(createdAt)));
  }

  json.Append("]");
  aResult = json;
  return NS_OK;
}

NS_IMETHODIMP
AIEngine::GetModels(nsACString& aResult) {
  nsCOMPtr<nsIURI> uri;
  nsresult rv = NS_NewURI(getter_AddRefs(uri),
                          "http://127.0.0.1:11434/api/tags"_ns);
  NS_ENSURE_SUCCESS(rv, rv);

  nsCOMPtr<nsIChannel> channel;
  rv = NS_NewChannel(getter_AddRefs(channel), uri,
                     nsContentUtils::GetSystemPrincipal(),
                     nsILoadInfo::SEC_ALLOW_CROSS_ORIGIN_SEC_CONTEXT_IS_NULL,
                     nsIContentPolicy::TYPE_OTHER);
  NS_ENSURE_SUCCESS(rv, rv);

  nsCOMPtr<nsIHttpChannel> httpChannel = do_QueryInterface(channel, &rv);
  NS_ENSURE_SUCCESS(rv, rv);

  rv = httpChannel->SetRequestMethod("GET"_ns);
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
AIEngine::SetModel(const nsACString& aModel) {
  mModel = aModel;
  return NS_OK;
}

NS_IMETHODIMP
AIEngine::GetModel(nsACString& aResult) {
  aResult = mModel;
  return NS_OK;
}

}  // namespace aiengine
}  // namespace mozilla
