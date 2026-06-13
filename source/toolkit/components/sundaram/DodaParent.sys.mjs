/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

const DODA_CONTRACT = "@mozilla.org/sundaram/service;1";

export class DodaParent extends JSWindowActorParent {
  receiveMessage(msg) {
    switch (msg.name) {
      case "Doda:ExecuteCommand": {
        try {
          const svc = Cc[DODA_CONTRACT].getService(Ci.nsIDodaService);
          const { command, args } = msg.data;
          const result = svc.executeCommand(command, args || "");
          this.sendAsyncMessage("Doda:CommandResult", { result });
        } catch (e) {
          this.sendAsyncMessage("Doda:CommandResult", { error: e.message });
        }
        break;
      }
    }
  }
}
