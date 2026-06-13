/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "IdentityWallet.h"
#include "nsIDirectoryService.h"
#include "nsAppDirectoryServiceDefs.h"
#include "nsServiceManagerUtils.h"
#include "mozIStorageService.h"
#include "mozIStorageStatement.h"
#include "nsPrintfCString.h"
#include "nsComponentManagerUtils.h"

// NSS crypto headers
#include "seccomon.h"
#include "secitem.h"
#include "keyhi.h"
#include "pk11pub.h"
#include "pk11priv.h"
#include "cryptohi.h"
#include "base64.h"
#include "hasht.h"
#include "secerr.h"
#include "nspr.h"

namespace mozilla {
namespace identitywallet {

NS_IMPL_ISUPPORTS(IdentityWallet, nsIIdentityWalletService)

IdentityWallet::IdentityWallet() : mInitialized(false) {}

IdentityWallet::~IdentityWallet() {
  if (mConnection) {
    (void) mConnection->Close();
  }
}

nsresult IdentityWallet::GetDatabasePath(nsIFile** aPath) {
  nsCOMPtr<nsIFile> profileDir;
  nsresult rv = NS_GetSpecialDirectory(NS_APP_USER_PROFILE_50_DIR,
                                       getter_AddRefs(profileDir));
  NS_ENSURE_SUCCESS(rv, rv);

  rv = profileDir->AppendNative("identity-wallet.sqlite"_ns);
  NS_ENSURE_SUCCESS(rv, rv);

  profileDir.forget(aPath);
  return NS_OK;
}

nsresult IdentityWallet::EnsureDatabase() {
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

nsresult IdentityWallet::CreateSchema() {
  nsresult rv;

  rv = mConnection->ExecuteSimpleSQL(
      "CREATE TABLE IF NOT EXISTS identities ("
      "  did TEXT PRIMARY KEY,"
      "  name TEXT NOT NULL DEFAULT '',"
      "  public_key BLOB NOT NULL,"
      "  private_key BLOB NOT NULL,"
      "  created_at TEXT NOT NULL DEFAULT (datetime('now'))"
      ")"_ns);
  NS_ENSURE_SUCCESS(rv, rv);

  rv = mConnection->ExecuteSimpleSQL(
      "CREATE TABLE IF NOT EXISTS site_permissions ("
      "  site TEXT NOT NULL,"
      "  did TEXT NOT NULL,"
      "  granted_at TEXT NOT NULL DEFAULT (datetime('now')),"
      "  PRIMARY KEY (site, did)"
      ")"_ns);
  NS_ENSURE_SUCCESS(rv, rv);

  rv = mConnection->ExecuteSimpleSQL(
      "CREATE TABLE IF NOT EXISTS active_logins ("
      "  site TEXT PRIMARY KEY,"
      "  did TEXT NOT NULL,"
      "  logged_in_at TEXT NOT NULL DEFAULT (datetime('now'))"
      ")"_ns);
  NS_ENSURE_SUCCESS(rv, rv);

  return NS_OK;
}

nsresult IdentityWallet::GenerateEd25519Key(nsACString& aPrivateKey,
                                            nsACString& aPublicKey,
                                            nsACString& aDID) {
  // Generate Ed25519 key pair using NSS
  PK11SlotInfo* slot = PK11_GetInternalSlot();
  if (!slot) {
    return NS_ERROR_FAILURE;
  }

  SECKEYPublicKey* pubKey = nullptr;
  SECKEYPrivateKey* privKey = nullptr;

  PK11RSAGenParams rsaParams;
  rsaParams.keySizeInBits = 2048;

  SECOidData* oidData = SECOID_FindOIDByTag(SEC_OID_CURVE25519);
  if (!oidData) {
    PK11_FreeSlot(slot);
    return NS_ERROR_FAILURE;
  }

  SECItem params;
  params.type = siBuffer;
  params.data = oidData->oid.data;
  params.len = oidData->oid.len;

  privKey = PK11_GenerateKeyPair(slot, CKM_EC_KEY_PAIR_GEN, &params,
                                  &pubKey, PR_FALSE, PR_TRUE, nullptr);
  PK11_FreeSlot(slot);

  if (!privKey || !pubKey) {
    if (privKey) SECKEY_DestroyPrivateKey(privKey);
    if (pubKey) SECKEY_DestroyPublicKey(pubKey);
    return NS_ERROR_FAILURE;
  }

  // Export private key (PKCS#8 DER)
  SECItem* privKeyItem = PK11_ExportDERPrivateKeyInfo(privKey, nullptr);
  if (!privKeyItem) {
    SECKEY_DestroyPrivateKey(privKey);
    SECKEY_DestroyPublicKey(pubKey);
    return NS_ERROR_FAILURE;
  }

  // Export public key (raw)
  SECItem pubKeyBytes = pubKey->u.ec.publicValue;

  // Base64url encode both
  char* privB64 = BTOA_ConvertItemToAscii(privKeyItem);
  char* pubB64 = BTOA_ConvertItemToAscii(&pubKeyBytes);

  aPrivateKey.Assign(privB64);
  aPublicKey.Assign(pubB64);

  // Derive DID: sha256(publicKey) → base64url prefix
  SECItem hashItem;
  hashItem.data = nullptr;
  hashItem.len = SHA256_LENGTH;
  PK11_HashBuf(SEC_OID_SHA256, hashItem.data, pubKeyBytes.data, pubKeyBytes.len);

  char* idB64 = BTOA_ConvertItemToAscii(&hashItem);
  aDID = nsPrintfCString("did:key:z%s", idB64);

  PORT_Free(idB64);
    PORT_Free(privB64);
    PORT_Free(pubB64);
  SECITEM_FreeItem(privKeyItem, PR_TRUE);
    SECKEY_DestroyPrivateKey(privKey);
    SECKEY_DestroyPublicKey(pubKey);
  return NS_OK;
}

nsresult IdentityWallet::KeyToDID(const nsACString& aPublicKey,
                                  nsACString& aDID) {
  // Decode base64 public key
  SECItem pubKeyItem;
  pubKeyItem.data = nullptr;
  pubKeyItem.len = 0;
  if (ATOB_ConvertAsciiToItem(&pubKeyItem, PromiseFlatCString(aPublicKey).get()) != SECSuccess) {
    return NS_ERROR_FAILURE;
  }

  // SHA-256 hash
  SECItem hashItem;
  hashItem.data = (unsigned char*)PORT_Alloc(SHA256_LENGTH);
  hashItem.len = SHA256_LENGTH;
  PK11_HashBuf(SEC_OID_SHA256, hashItem.data, pubKeyItem.data, pubKeyItem.len);

  char* idB64 = BTOA_ConvertItemToAscii(&hashItem);
  aDID = nsPrintfCString("did:key:z%s", idB64);

  PORT_Free(idB64);
  PORT_Free(hashItem.data);
  SECITEM_FreeItem(&pubKeyItem, PR_FALSE);
  return NS_OK;
}

// --- IDL Implementation ---

NS_IMETHODIMP
IdentityWallet::CreateIdentity(const nsACString& aName, nsACString& aResult) {
  nsresult rv = EnsureDatabase();
  NS_ENSURE_SUCCESS(rv, rv);

  nsAutoCString privKey, pubKey, did;
  rv = GenerateEd25519Key(privKey, pubKey, did);
      NS_ENSURE_SUCCESS(rv, rv);

  nsCOMPtr<mozIStorageStatement> stmt;
  rv = mConnection->CreateStatement(
      "INSERT INTO identities (did, name, public_key, private_key) "
      "VALUES (?1, ?2, ?3, ?4)"_ns,
      getter_AddRefs(stmt));
  NS_ENSURE_SUCCESS(rv, rv);

  rv = stmt->BindUTF8StringByIndex(0, did);
  NS_ENSURE_SUCCESS(rv, rv);
  rv = stmt->BindUTF8StringByIndex(1, aName);
  NS_ENSURE_SUCCESS(rv, rv);
  rv = stmt->BindUTF8StringByIndex(2, pubKey);
  NS_ENSURE_SUCCESS(rv, rv);
  rv = stmt->BindUTF8StringByIndex(3, privKey);
  NS_ENSURE_SUCCESS(rv, rv);

  rv = stmt->Execute();
  NS_ENSURE_SUCCESS(rv, rv);

  // Return JSON
  aResult = nsPrintfCString(
      "{\"did\":\"%s\",\"name\":\"%s\"}", did.get(), PromiseFlatCString(aName).get());
    return NS_OK;
}

NS_IMETHODIMP
IdentityWallet::ListIdentities(nsACString& aResult) {
  nsresult rv = EnsureDatabase();
  NS_ENSURE_SUCCESS(rv, rv);

  nsCOMPtr<mozIStorageStatement> stmt;
  rv = mConnection->CreateStatement(
      "SELECT did, name, created_at FROM identities ORDER BY created_at DESC"_ns,
      getter_AddRefs(stmt));
  NS_ENSURE_SUCCESS(rv, rv);

  nsAutoCString json("[");
  bool first = true;
  bool hasMore;

  while (NS_SUCCEEDED(stmt->ExecuteStep(&hasMore)) && hasMore) {
    if (!first) json.Append(",");
    first = false;

    nsAutoCString did, name, created;
    (void) stmt->GetUTF8String(0, did);
    (void) stmt->GetUTF8String(1, name);
    (void) stmt->GetUTF8String(2, created);

    json.Append(nsPrintfCString(
        "{\"did\":\"%s\",\"name\":\"%s\",\"created_at\":\"%s\"}",
        did.get(), name.get(), created.get()));
  }

  json.Append("]");
  aResult = json;
  return NS_OK;
}

NS_IMETHODIMP
IdentityWallet::DeleteIdentity(const nsACString& aDID) {
  nsresult rv = EnsureDatabase();
      NS_ENSURE_SUCCESS(rv, rv);

  nsCOMPtr<mozIStorageStatement> stmt;
  rv = mConnection->CreateStatement(
      "DELETE FROM identities WHERE did = ?1"_ns,
      getter_AddRefs(stmt));
  NS_ENSURE_SUCCESS(rv, rv);

  rv = stmt->BindUTF8StringByIndex(0, aDID);
  NS_ENSURE_SUCCESS(rv, rv);

  rv = stmt->Execute();
  NS_ENSURE_SUCCESS(rv, rv);

  // Clean up permissions and logins
  nsCOMPtr<mozIStorageStatement> cleanup;
  rv = mConnection->CreateStatement(
      "DELETE FROM site_permissions WHERE did = ?1"_ns,
      getter_AddRefs(cleanup));
  NS_ENSURE_SUCCESS(rv, rv);
  cleanup->BindUTF8StringByIndex(0, aDID);
  (void) cleanup->Execute();

  rv = mConnection->CreateStatement(
      "DELETE FROM active_logins WHERE did = ?1"_ns,
      getter_AddRefs(cleanup));
  NS_ENSURE_SUCCESS(rv, rv);
  cleanup->BindUTF8StringByIndex(0, aDID);
  (void) cleanup->Execute();

  return NS_OK;
}

NS_IMETHODIMP
IdentityWallet::Sign(const nsACString& aDID, const nsACString& aMessage,
                     nsACString& aResult) {
  nsresult rv = EnsureDatabase();
  NS_ENSURE_SUCCESS(rv, rv);

  // Load private key from database
  nsCOMPtr<mozIStorageStatement> stmt;
  rv = mConnection->CreateStatement(
      "SELECT private_key FROM identities WHERE did = ?1"_ns,
      getter_AddRefs(stmt));
  NS_ENSURE_SUCCESS(rv, rv);

  rv = stmt->BindUTF8StringByIndex(0, aDID);
  NS_ENSURE_SUCCESS(rv, rv);

  bool hasMore;
  rv = stmt->ExecuteStep(&hasMore);
  NS_ENSURE_SUCCESS(rv, rv);

  if (!hasMore) {
    return NS_ERROR_NOT_AVAILABLE;
  }

  nsAutoCString privKeyB64;
  rv = stmt->GetUTF8String(0, privKeyB64);
  NS_ENSURE_SUCCESS(rv, rv);

  // Decode private key
  SECItem privKeyItem;
  privKeyItem.data = nullptr;
  privKeyItem.len = 0;
  if (ATOB_ConvertAsciiToItem(&privKeyItem, privKeyB64.get()) != SECSuccess) {
    return NS_ERROR_FAILURE;
  }

  // Import private key
  SECKEYPrivateKey* privKey = nullptr;
  SECStatus srv = PK11_ImportDERPrivateKeyInfoAndReturnKey(
      PK11_GetInternalSlot(), &privKeyItem, nullptr, nullptr, PR_FALSE,
      PR_FALSE, KU_ALL, &privKey, nullptr);
  SECITEM_FreeItem(&privKeyItem, PR_FALSE);

  if (srv != SECSuccess || !privKey) {
    return NS_ERROR_FAILURE;
  }

  // Sign with EdDSA
  SECItem signature;
  signature.data = nullptr;
  signature.len = 0;

  SECItem msgItem;
  msgItem.data = (unsigned char*)PromiseFlatCString(aMessage).get();
  msgItem.len = aMessage.Length();

  if (PK11_SignWithMechanism(privKey, CKM_EDDSA, nullptr, &signature,
                              &msgItem) != SECSuccess) {
    SECKEY_DestroyPrivateKey(privKey);
    return NS_ERROR_FAILURE;
  }

  // Base64 encode signature
  char* sigB64 = BTOA_ConvertItemToAscii(&signature);
  aResult.Assign(sigB64);

  PORT_Free(sigB64);
  SECITEM_FreeItem(&signature, PR_FALSE);
  SECKEY_DestroyPrivateKey(privKey);
  return NS_OK;
}

NS_IMETHODIMP
IdentityWallet::Verify(const nsACString& aDID, const nsACString& aMessage,
                       const nsACString& aSignature, bool* aResult) {
  *aResult = false;

  nsresult rv = EnsureDatabase();
  NS_ENSURE_SUCCESS(rv, rv);

  // Load public key
  nsCOMPtr<mozIStorageStatement> stmt;
  rv = mConnection->CreateStatement(
      "SELECT public_key FROM identities WHERE did = ?1"_ns,
      getter_AddRefs(stmt));
  NS_ENSURE_SUCCESS(rv, rv);

  rv = stmt->BindUTF8StringByIndex(0, aDID);
  NS_ENSURE_SUCCESS(rv, rv);

  bool hasMore;
  rv = stmt->ExecuteStep(&hasMore);
  NS_ENSURE_SUCCESS(rv, rv);

  if (!hasMore) {
    return NS_OK;
  }

  nsAutoCString pubKeyB64;
  rv = stmt->GetUTF8String(0, pubKeyB64);
  NS_ENSURE_SUCCESS(rv, rv);

  // Decode public key
  SECItem pubKeyItem;
  pubKeyItem.data = nullptr;
  pubKeyItem.len = 0;
  if (ATOB_ConvertAsciiToItem(&pubKeyItem, pubKeyB64.get()) != SECSuccess) {
    return NS_OK;
  }

  // Import public key
  SECKEYPublicKey* pubKey = SECKEY_ImportDERPublicKey(&pubKeyItem, SEC_OID_CURVE25519);
  SECITEM_FreeItem(&pubKeyItem, PR_FALSE);

  if (!pubKey) {
    return NS_OK;
  }

  // Verify signature
  SECItem signatureItem;
  signatureItem.data = nullptr;
  signatureItem.len = 0;
  if (ATOB_ConvertAsciiToItem(&signatureItem, PromiseFlatCString(aSignature).get()) != SECSuccess) {
    SECKEY_DestroyPublicKey(pubKey);
    return NS_OK;
  }

  SECItem msgItem;
  msgItem.data = (unsigned char*)PromiseFlatCString(aMessage).get();
  msgItem.len = aMessage.Length();

  SECStatus status = PK11_VerifyWithMechanism(
      pubKey, CKM_EDDSA, nullptr, &signatureItem, &msgItem, nullptr);
  *aResult = (status == SECSuccess);

  SECITEM_FreeItem(&signatureItem, PR_FALSE);
  SECKEY_DestroyPublicKey(pubKey);
  return NS_OK;
}

NS_IMETHODIMP
IdentityWallet::GrantSitePermission(const nsACString& aDID, const nsACString& aSite) {
  nsresult rv = EnsureDatabase();
  NS_ENSURE_SUCCESS(rv, rv);

  nsCOMPtr<mozIStorageStatement> stmt;
  rv = mConnection->CreateStatement(
      "INSERT OR REPLACE INTO site_permissions (site, did) VALUES (?1, ?2)"_ns,
      getter_AddRefs(stmt));
  NS_ENSURE_SUCCESS(rv, rv);

  rv = stmt->BindUTF8StringByIndex(0, aSite);
  NS_ENSURE_SUCCESS(rv, rv);
  rv = stmt->BindUTF8StringByIndex(1, aDID);
  NS_ENSURE_SUCCESS(rv, rv);

  rv = stmt->Execute();
  NS_ENSURE_SUCCESS(rv, rv);

  return NS_OK;
}

NS_IMETHODIMP
IdentityWallet::RevokeSitePermission(const nsACString& aDID, const nsACString& aSite) {
  nsresult rv = EnsureDatabase();
  NS_ENSURE_SUCCESS(rv, rv);

  nsCOMPtr<mozIStorageStatement> stmt;
  rv = mConnection->CreateStatement(
      "DELETE FROM site_permissions WHERE did = ?1 AND site = ?2"_ns,
      getter_AddRefs(stmt));
  NS_ENSURE_SUCCESS(rv, rv);

  rv = stmt->BindUTF8StringByIndex(0, aDID);
  NS_ENSURE_SUCCESS(rv, rv);
  rv = stmt->BindUTF8StringByIndex(1, aSite);
  NS_ENSURE_SUCCESS(rv, rv);

  rv = stmt->Execute();
  NS_ENSURE_SUCCESS(rv, rv);

  return NS_OK;
}

NS_IMETHODIMP
IdentityWallet::ListSitePermissions(nsACString& aResult) {
  nsresult rv = EnsureDatabase();
  NS_ENSURE_SUCCESS(rv, rv);

  nsCOMPtr<mozIStorageStatement> stmt;
  rv = mConnection->CreateStatement(
      "SELECT site, did, granted_at FROM site_permissions ORDER BY site"_ns,
      getter_AddRefs(stmt));
  NS_ENSURE_SUCCESS(rv, rv);

  nsAutoCString json("[");
  bool first = true;
  bool hasMore;

  while (NS_SUCCEEDED(stmt->ExecuteStep(&hasMore)) && hasMore) {
    if (!first) json.Append(",");
    first = false;

    nsAutoCString site, did, granted;
    (void) stmt->GetUTF8String(0, site);
    (void) stmt->GetUTF8String(1, did);
    (void) stmt->GetUTF8String(2, granted);

    json.Append(nsPrintfCString(
        "{\"site\":\"%s\",\"did\":\"%s\",\"granted_at\":\"%s\"}",
        site.get(), did.get(), granted.get()));
  }

  json.Append("]");
  aResult = json;
  return NS_OK;
}

NS_IMETHODIMP
IdentityWallet::GetActiveLogin(const nsACString& aSite, nsACString& aResult) {
  nsresult rv = EnsureDatabase();
  NS_ENSURE_SUCCESS(rv, rv);

  nsCOMPtr<mozIStorageStatement> stmt;
  rv = mConnection->CreateStatement(
      "SELECT did FROM active_logins WHERE site = ?1"_ns,
      getter_AddRefs(stmt));
  NS_ENSURE_SUCCESS(rv, rv);

  rv = stmt->BindUTF8StringByIndex(0, aSite);
  NS_ENSURE_SUCCESS(rv, rv);

  bool hasMore;
  rv = stmt->ExecuteStep(&hasMore);
  NS_ENSURE_SUCCESS(rv, rv);

  if (hasMore) {
    nsAutoCString did;
    stmt->GetUTF8String(0, did);
    aResult = did;
  } else {
    aResult.Truncate();
  }

  return NS_OK;
}

NS_IMETHODIMP
IdentityWallet::SetActiveLogin(const nsACString& aSite, const nsACString& aDID) {
  nsresult rv = EnsureDatabase();
  NS_ENSURE_SUCCESS(rv, rv);

  nsCOMPtr<mozIStorageStatement> stmt;
  rv = mConnection->CreateStatement(
      "INSERT OR REPLACE INTO active_logins (site, did) VALUES (?1, ?2)"_ns,
      getter_AddRefs(stmt));
  NS_ENSURE_SUCCESS(rv, rv);

  rv = stmt->BindUTF8StringByIndex(0, aSite);
  NS_ENSURE_SUCCESS(rv, rv);
  rv = stmt->BindUTF8StringByIndex(1, aDID);
  NS_ENSURE_SUCCESS(rv, rv);

  rv = stmt->Execute();
  NS_ENSURE_SUCCESS(rv, rv);

  return NS_OK;
}

NS_IMETHODIMP
IdentityWallet::ClearLogin(const nsACString& aSite) {
  nsresult rv = EnsureDatabase();
  NS_ENSURE_SUCCESS(rv, rv);

  nsCOMPtr<mozIStorageStatement> stmt;
  rv = mConnection->CreateStatement(
      "DELETE FROM active_logins WHERE site = ?1"_ns,
      getter_AddRefs(stmt));
  NS_ENSURE_SUCCESS(rv, rv);

  rv = stmt->BindUTF8StringByIndex(0, aSite);
  NS_ENSURE_SUCCESS(rv, rv);

  rv = stmt->Execute();
  NS_ENSURE_SUCCESS(rv, rv);

  return NS_OK;
}

NS_IMETHODIMP
IdentityWallet::GetLoginName(const nsACString& aDID, nsACString& aResult) {
  nsresult rv = EnsureDatabase();
  NS_ENSURE_SUCCESS(rv, rv);

  nsCOMPtr<mozIStorageStatement> stmt;
  rv = mConnection->CreateStatement(
      "SELECT name FROM identities WHERE did = ?1"_ns,
      getter_AddRefs(stmt));
  NS_ENSURE_SUCCESS(rv, rv);

  rv = stmt->BindUTF8StringByIndex(0, aDID);
  NS_ENSURE_SUCCESS(rv, rv);

  bool hasMore;
  rv = stmt->ExecuteStep(&hasMore);
  NS_ENSURE_SUCCESS(rv, rv);

  if (hasMore) {
    stmt->GetUTF8String(0, aResult);
  } else {
    aResult.Truncate();
  }

  return NS_OK;
}

NS_IMETHODIMP
IdentityWallet::SetLoginName(const nsACString& aDID, const nsACString& aName) {
  nsresult rv = EnsureDatabase();
  NS_ENSURE_SUCCESS(rv, rv);

  nsCOMPtr<mozIStorageStatement> stmt;
  rv = mConnection->CreateStatement(
      "UPDATE identities SET name = ?1 WHERE did = ?2"_ns,
      getter_AddRefs(stmt));
  NS_ENSURE_SUCCESS(rv, rv);

  rv = stmt->BindUTF8StringByIndex(0, aName);
  NS_ENSURE_SUCCESS(rv, rv);
  rv = stmt->BindUTF8StringByIndex(1, aDID);
  NS_ENSURE_SUCCESS(rv, rv);

  rv = stmt->Execute();
  NS_ENSURE_SUCCESS(rv, rv);

  return NS_OK;
}

}  // namespace identitywallet
}  // namespace mozilla
