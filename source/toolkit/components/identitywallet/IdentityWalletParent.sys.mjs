/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

export class IdentityWalletParent extends JSWindowActorParent {
  receiveMessage(msg) {
    switch (msg.name) {
      case "ID:GetActiveLogin": {
        try {
          const svc = Cc["@mozilla.org/identitywallet/service;1"]
            .getService(Ci.nsIIdentityWalletService);
          const did = svc.getActiveLogin(msg.data.site);
          this.sendAsyncMessage("ID:ActiveLoginResult", { did });
        } catch (e) {
          console.warn("IdentityWalletParent: getActiveLogin failed", e);
        }
        break;
      }
    }
  }
}
