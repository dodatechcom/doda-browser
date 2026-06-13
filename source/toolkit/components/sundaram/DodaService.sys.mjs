/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

const DODA_CONTRACT = "@mozilla.org/sundaram/service;1";

export class DodaService {
  static getService() {
    if (!this._service) {
      this._service = Cc[DODA_CONTRACT].getService(Ci.nsIDodaService);
    }
    return this._service;
  }

  static search(query) {
    return JSON.parse(this.getService().search(query));
  }

  static getStats() {
    return JSON.parse(this.getService().getStats());
  }

  static executeCommand(command, args = "") {
    return this.getService().executeCommand(command, args);
  }

  static getCommands() {
    return JSON.parse(this.getService().getCommands());
  }
}
