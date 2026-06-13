/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef IdentityWallet_h_
#define IdentityWallet_h_

#include "nsIIdentityWalletService.h"
#include "nsCOMPtr.h"
#include "nsIFile.h"
#include "nsString.h"
#include "mozIStorageConnection.h"
#include "mozIStorageService.h"
#include "mozilla/Attributes.h"

namespace mozilla {
namespace identitywallet {

class IdentityWallet final : public nsIIdentityWalletService {
 public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIIDENTITYWALLETSERVICE

  IdentityWallet();

 private:
  ~IdentityWallet();

  nsresult EnsureDatabase();
  nsresult GetDatabasePath(nsIFile** aPath);
  nsresult CreateSchema();
  nsresult GenerateEd25519Key(nsACString& aPrivateKey, nsACString& aPublicKey,
                              nsACString& aDID);
  nsresult KeyToDID(const nsACString& aPublicKey, nsACString& aDID);

  nsCOMPtr<mozIStorageConnection> mConnection;
  bool mInitialized;
};

}  // namespace identitywallet
}  // namespace mozilla

#endif  // IdentityWallet_h_
