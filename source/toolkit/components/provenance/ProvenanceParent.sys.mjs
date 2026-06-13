/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

const PV_CONTRACT = "@mozilla.org/provenance/service;1";

export class ProvenanceParent extends JSWindowActorParent {
  receiveMessage(msg) {
    switch (msg.name) {
      case "PV:ExtractClaims": {
        try {
          const svc = Cc[PV_CONTRACT].getService(Ci.nsIProvenanceService);
          const { url, title, textContent } = msg.data;
          const result = svc.extractClaims(url, title || "", textContent || "");
          this.sendAsyncMessage("PV:ClaimsResult", { result });
        } catch (e) {
          console.warn("ProvenanceParent: extractClaims failed", e);
          this.sendAsyncMessage("PV:ClaimsResult", { error: e.message });
        }
        break;
      }
    }
  }
}
