#ifndef Provenance_h_
#define Provenance_h_

#include "nsIProvenanceService.h"
#include "nsCOMPtr.h"
#include "nsString.h"
#include "nsIFile.h"
#include "mozIStorageConnection.h"
#include "mozilla/Attributes.h"

namespace mozilla {
namespace provenance {

class Provenance final : public nsIProvenanceService {
 public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIPROVENANCESERVICE

  Provenance();

 private:
  ~Provenance();

  nsresult EnsureDatabase();
  nsresult GetDatabasePath(nsIFile** aPath);
  nsresult CreateSchema();
  nsresult OllamaRequest(const nsACString& aEndpoint, const nsACString& aBody, nsACString& aResult);
  nsresult ParseClaimsFromResponse(const nsACString& aResponse, const nsACString& aUrl, const nsACString& aTitle);
  nsresult StoreClaim(const nsACString& aText, const nsACString& aUrl, const nsACString& aTitle, const nsACString& aDomain);

  nsCOMPtr<mozIStorageConnection> mConnection;
  bool mInitialized;
};

}  // namespace provenance
}  // namespace mozilla

#endif  // Provenance_h_
