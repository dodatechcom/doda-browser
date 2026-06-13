/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef KnowledgeGraph_h_
#define KnowledgeGraph_h_

#include "nsIKnowledgeGraphService.h"
#include "nsCOMPtr.h"
#include "nsIFile.h"
#include "nsString.h"
#include "mozIStorageConnection.h"
#include "mozIStorageService.h"
#include "mozilla/Attributes.h"

namespace mozilla {
namespace knowledgegraph {

class KnowledgeGraph final : public nsIKnowledgeGraphService {
 public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIKNOWLEDGEGRAPHSERVICE

  KnowledgeGraph();

 private:
  ~KnowledgeGraph();

  nsresult EnsureDatabase();
  nsresult GetDatabasePath(nsIFile** aPath);
  nsresult CreateSchema();
  nsresult ToJSON(const char* aFormat, ...);

  nsCOMPtr<mozIStorageConnection> mConnection;
  bool mInitialized;
};

}  // namespace knowledgegraph
}  // namespace mozilla

#endif  // KnowledgeGraph_h_
