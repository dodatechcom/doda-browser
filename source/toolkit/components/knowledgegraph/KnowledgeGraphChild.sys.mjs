/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

export class KnowledgeGraphChild extends JSWindowActorChild {
  handleEvent(event) {
    if (event.type === "DOMContentLoaded") {
      this._scheduleIndex();
    }
  }

  _scheduleIndex() {
    // Use idle callback to avoid impacting page load performance
    this.contentWindow.requestIdleCallback(() => {
      this._indexPage();
    }, { timeout: 5000 });
  }

  _indexPage() {
    const doc = this.document;
    if (!doc || !doc.body || !doc.location) return;

    // Skip non-http(s) pages
    const url = doc.location.href;
    if (!url.startsWith("http://") && !url.startsWith("https://")) return;

    // Extract text content from the page body
    const textContent = doc.body.textContent || "";
    if (textContent.length < 50) return; // Skip near-empty pages

    const title = doc.title || url;
    const visitedAt = new Date().toISOString();

    this.sendAsyncMessage("KG:IndexPage", {
      url,
      title,
      textContent: textContent.slice(0, 50000), // Cap at 50K chars
      visitedAt,
    });
  }
}
