/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

const ID_CONTRACT = "@mozilla.org/identitywallet/service;1";

export class IdentityWallet {
  static getService() {
    if (!this._service) {
      this._service = Cc[ID_CONTRACT].getService(Ci.nsIIdentityWalletService);
    }
    return this._service;
  }

  static createIdentity(name) {
    return JSON.parse(this.getService().createIdentity(name));
  }

  static listIdentities() {
    return JSON.parse(this.getService().listIdentities());
  }

  static deleteIdentity(did) {
    this.getService().deleteIdentity(did);
  }

  static sign(did, message) {
    return this.getService().sign(did, message);
  }

  static verify(did, message, signature) {
    return this.getService().verify(did, message, signature);
  }

  static grantSitePermission(did, site) {
    this.getService().grantSitePermission(did, site);
  }

  static revokeSitePermission(did, site) {
    this.getService().revokeSitePermission(did, site);
  }

  static listSitePermissions() {
    return JSON.parse(this.getService().listSitePermissions());
  }

  static getActiveLogin(site) {
    return this.getService().getActiveLogin(site);
  }

  static setActiveLogin(site, did) {
    this.getService().setActiveLogin(site, did);
  }

  static clearLogin(site) {
    this.getService().clearLogin(site);
  }
}
