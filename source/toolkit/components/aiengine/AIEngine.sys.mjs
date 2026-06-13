/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

const AI_CONTRACT = "@mozilla.org/aiengine/service;1";

export class AIEngine {
  static getService() {
    if (!this._service) {
      this._service = Cc[AI_CONTRACT].getService(Ci.nsIAIEngineService);
    }
    return this._service;
  }

  static checkHealth() {
    return this.getService().checkHealth();
  }

  static chat(message, history) {
    return this.getService().chat(message, history || "");
  }

  static analyzeTab(pageContent, prompt) {
    return this.getService().analyzeTab(pageContent, prompt || "");
  }

  static crossPage(pages, prompt) {
    return this.getService().crossPage(pages, prompt || "");
  }

  static debate(topic, perspectives) {
    return this.getService().debate(topic, perspectives || "");
  }

  static clearHistory() {
    this.getService().clearHistory();
  }

  static getHistory(limit = 50) {
    return JSON.parse(this.getService().getHistory(limit));
  }

  static getModels() {
    return JSON.parse(this.getService().getModels());
  }

  static setModel(model) {
    this.getService().setModel(model);
  }

  static getModel() {
    return this.getService().getModel();
  }
}
