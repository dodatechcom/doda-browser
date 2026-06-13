/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

const KG_CONTRACT = "@mozilla.org/knowledgegraph/service;1";

export class KnowledgeGraph {
  static getService() {
    if (!this._service) {
      this._service = Cc[KG_CONTRACT].getService(Ci.nsIKnowledgeGraphService);
    }
    return this._service;
  }

  static async indexPage(url, title, textContent, visitedAt) {
    try {
      const svc = this.getService();
      svc.indexPage(url, title || "", textContent || "", visitedAt || new Date().toISOString());
      return true;
    } catch (e) {
      console.warn("KnowledgeGraph: indexPage failed", e);
      return false;
    }
  }

  static search(query, maxResults = 50) {
    const svc = this.getService();
    return JSON.parse(svc.search(query, maxResults));
  }

  static getRelated(url, maxResults = 20) {
    const svc = this.getService();
    return JSON.parse(svc.getRelated(url, maxResults));
  }

  static getRecent(maxResults = 100) {
    const svc = this.getService();
    return JSON.parse(svc.getRecent(maxResults));
  }

  static getPageCount() {
    return this.getService().getPageCount();
  }

  static clear() {
    this.getService().clear();
  }
}
