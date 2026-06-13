/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

const PV_CONTRACT = "@mozilla.org/provenance/service;1";

export class Provenance {
  static getService() {
    if (!this._service) {
      this._service = Cc[PV_CONTRACT].getService(Ci.nsIProvenanceService);
    }
    return this._service;
  }

  static extractClaims(url, title, textContent) {
    return JSON.parse(this.getService().extractClaims(url, title || "", textContent || ""));
  }

  static compareSources(claimId) {
    return JSON.parse(this.getService().compareSources(claimId));
  }

  static findSpread(claimId, maxResults = 50) {
    return JSON.parse(this.getService().findSpread(claimId, maxResults));
  }

  static searchClaims(query, maxResults = 50) {
    return JSON.parse(this.getService().searchClaims(query, maxResults));
  }

  static getRecent(maxResults = 50) {
    return JSON.parse(this.getService().getRecent(maxResults));
  }

  static getClaimCount() {
    return this.getService().getClaimCount();
  }

  static clear() {
    this.getService().clear();
  }
}
