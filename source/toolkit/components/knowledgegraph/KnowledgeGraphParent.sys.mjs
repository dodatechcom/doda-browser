/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

const KG_CONTRACT = "@mozilla.org/knowledgegraph/service;1";

export class KnowledgeGraphParent extends JSWindowActorParent {
  receiveMessage(msg) {
    switch (msg.name) {
      case "KG:IndexPage": {
        try {
          const svc = Cc[KG_CONTRACT].getService(Ci.nsIKnowledgeGraphService);
          const { url, title, textContent, visitedAt } = msg.data;
          svc.indexPage(url, title || "", textContent || "", visitedAt || new Date().toISOString());
        } catch (e) {
          console.warn("KnowledgeGraphParent: index failed", e);
        }
        break;
      }
    }
  }
}
