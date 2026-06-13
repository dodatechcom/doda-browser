/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

export class ProvenanceChild extends JSWindowActorChild {
  handleEvent(event) {
    if (event.type === "DOMContentLoaded") {
      this._scheduleExtraction();
    }
  }

  _scheduleExtraction() {
    this.contentWindow.requestIdleCallback(() => {
      this._extractClaims();
    }, { timeout: 10000 });
  }

  _extractClaims() {
    const doc = this.document;
    if (!doc || !doc.body || !doc.location) return;

    const url = doc.location.href;
    if (!url.startsWith("http://") && !url.startsWith("https://")) return;

    const textContent = doc.body.textContent || "";
    if (textContent.length < 200) return; // Skip short pages

    this.sendAsyncMessage("PV:ExtractClaims", {
      url,
      title: doc.title || url,
      textContent: textContent.slice(0, 100000),
    });
  }
}
