#include "DodaService.h"
#include "nsIKnowledgeGraphService.h"
#include "nsIProvenanceService.h"
#include "nsIAIEngineService.h"
#include "nsIIdentityWalletService.h"
#include "nsServiceManagerUtils.h"
#include "nsPrintfCString.h"
#include "nsReadableUtils.h"
#include "mozilla/Components.h"

namespace mozilla {
namespace sundaram {

NS_IMPL_ISUPPORTS(DodaService, nsIDodaService)

DodaService::DodaService() {}

DodaService::~DodaService() {}

NS_IMETHODIMP
DodaService::Search(const nsACString& aQuery, nsACString& aResult) {
  // Unified search across all components
  nsAutoCString json("{\"results\":[");
  bool first = true;

  // Search Knowledge Graph
  nsCOMPtr<nsIKnowledgeGraphService> kg =
      do_GetService("@mozilla.org/knowledgegraph/service;1");
  if (kg) {
    nsAutoCString kgResult;
    nsresult rv = kg->Search(aQuery, 10, kgResult);
    if (NS_SUCCEEDED(rv)) {
      if (!first) json.Append(",");
      first = false;
      json.Append(nsPrintfCString(
          "{\"source\":\"knowledgegraph\",\"results\":%s}", kgResult.get()));
    }
  }

  // Search Provenance
  nsCOMPtr<nsIProvenanceService> pv =
      do_GetService("@mozilla.org/provenance/service;1");
  if (pv) {
    nsAutoCString pvResult;
    nsresult rv = pv->SearchClaims(aQuery, 10, pvResult);
    if (NS_SUCCEEDED(rv)) {
      if (!first) json.Append(",");
      first = false;
      json.Append(nsPrintfCString(
          "{\"source\":\"provenance\",\"results\":%s}", pvResult.get()));
    }
  }

  json.Append("]}");
  aResult = json;
  return NS_OK;
}

NS_IMETHODIMP
DodaService::GetStats(nsACString& aResult) {
  uint32_t pageCount = 0;
  uint32_t claimCount = 0;
  uint32_t identityCount = 0;

  nsCOMPtr<nsIKnowledgeGraphService> kg =
      do_GetService("@mozilla.org/knowledgegraph/service;1");
  if (kg) {
    (void) kg->GetPageCount(&pageCount);
  }

  nsCOMPtr<nsIProvenanceService> pv =
      do_GetService("@mozilla.org/provenance/service;1");
  if (pv) {
    (void) pv->GetClaimCount(&claimCount);
  }

  nsCOMPtr<nsIIdentityWalletService> id =
      do_GetService("@mozilla.org/identitywallet/service;1");
  if (id) {
    nsAutoCString ids;
    (void) id->ListIdentities(ids);
    // Count identities from JSON array
    if (!ids.IsEmpty()) {
      int32_t count = 0;
      int32_t pos = 0;
      while (true) {
        int32_t idx = ids.Find("{", pos);
        if (idx == -1) break;
        count++;
        pos = idx + 1;
      }
      identityCount = count;
    }
  }

  aResult = nsPrintfCString(
      "{\"pages_indexed\":%u,\"claims_extracted\":%u,\"identities\":%u}",
      pageCount, claimCount, identityCount);
  return NS_OK;
}

NS_IMETHODIMP
DodaService::ExecuteCommand(const nsACString& aCommand,
                            const nsACString& aArgs, nsACString& aResult) {
  if (aCommand.EqualsLiteral("kg:search")) {
    nsCOMPtr<nsIKnowledgeGraphService> kg =
        do_GetService("@mozilla.org/knowledgegraph/service;1");
    if (kg) {
      nsAutoCString result;
      kg->Search(aArgs, 10, result);
      aResult = result;
    }
    return NS_OK;
  }

  if (aCommand.EqualsLiteral("pv:search")) {
    nsCOMPtr<nsIProvenanceService> pv =
        do_GetService("@mozilla.org/provenance/service;1");
    if (pv) {
      nsAutoCString result;
      pv->SearchClaims(aArgs, 10, result);
      aResult = result;
    }
    return NS_OK;
  }

  if (aCommand.EqualsLiteral("ai:chat")) {
    nsCOMPtr<nsIAIEngineService> ai =
        do_GetService("@mozilla.org/aiengine/service;1");
    if (ai) {
      ai->Chat(aArgs, ""_ns, aResult);
    }
    return NS_OK;
  }

  if (aCommand.EqualsLiteral("id:create")) {
    nsCOMPtr<nsIIdentityWalletService> id =
        do_GetService("@mozilla.org/identitywallet/service;1");
    if (id) {
      id->CreateIdentity(aArgs, aResult);
    }
    return NS_OK;
  }

  if (aCommand.EqualsLiteral("kg:clear")) {
    nsCOMPtr<nsIKnowledgeGraphService> kg =
        do_GetService("@mozilla.org/knowledgegraph/service;1");
    if (kg) {
      kg->Clear();
      aResult = "{\"ok\":true}";
    }
    return NS_OK;
  }

  if (aCommand.EqualsLiteral("pv:clear")) {
    nsCOMPtr<nsIProvenanceService> pv =
        do_GetService("@mozilla.org/provenance/service;1");
    if (pv) {
      pv->Clear();
      aResult = "{\"ok\":true}";
    }
    return NS_OK;
  }

  if (aCommand.EqualsLiteral("ai:clear")) {
    nsCOMPtr<nsIAIEngineService> ai =
        do_GetService("@mozilla.org/aiengine/service;1");
    if (ai) {
      ai->ClearHistory();
      aResult = "{\"ok\":true}";
    }
    return NS_OK;
  }

  aResult = nsPrintfCString("{\"error\":\"unknown command: %s\"}", PromiseFlatCString(aCommand).get());
  return NS_OK;
}

static const char* const kCommands[] = {
    "kg:search",  "kg:clear",   "pv:search", "pv:clear",
    "ai:chat",    "ai:clear",   "id:create", nullptr};

NS_IMETHODIMP
DodaService::GetCommands(nsACString& aResult) {
  nsAutoCString json("[");
  bool first = true;
  for (int32_t i = 0; kCommands[i] != nullptr; i++) {
    if (!first) json.Append(",");
    first = false;
    json.Append(nsPrintfCString("\"%s\"", kCommands[i]));
  }
  json.Append("]");
  aResult = json;
  return NS_OK;
}

}  // namespace sundaram
}  // namespace mozilla
