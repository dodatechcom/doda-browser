#ifndef AIEngine_h_
#define AIEngine_h_

#include "nsIAIEngineService.h"
#include "nsCOMPtr.h"
#include "nsString.h"
#include "nsIFile.h"
#include "mozIStorageConnection.h"
#include "mozilla/Attributes.h"

namespace mozilla {
namespace aiengine {

class AIEngine final : public nsIAIEngineService {
 public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIAIENGINESERVICE

  AIEngine();

 private:
  ~AIEngine();

  nsresult EnsureDatabase();
  nsresult GetDatabasePath(nsIFile** aPath);
  nsresult CreateSchema();
  nsresult OllamaRequest(const nsACString& aEndpoint, const nsACString& aBody, nsACString& aResult);
  nsresult ToJSON(const char* aFormat, ...);

  nsCOMPtr<mozIStorageConnection> mConnection;
  nsCString mModel;
  bool mInitialized;
};

}  // namespace aiengine
}  // namespace mozilla

#endif  // AIEngine_h_
