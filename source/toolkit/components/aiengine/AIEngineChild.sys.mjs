/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

export class AIEngineChild extends JSWindowActorChild {
  handleEvent(event) {
    if (event.type === "DOMContentLoaded") {
      this._injectAIActions();
    }
  }

  _injectAIActions() {
    const doc = this.document;
    if (!doc || !doc.body) return;

    // Right-click context menu integration placeholder
    // Future: add "Ask AI about this" to context menu
  }

  async analyzeCurrentTab(prompt) {
    const doc = this.document;
    if (!doc || !doc.body) return null;

    const textContent = doc.body.textContent || "";
    const pageContent = textContent.slice(0, 50000);

    this.sendAsyncMessage("AI:AnalyzeTab", { pageContent, prompt });
  }
}
