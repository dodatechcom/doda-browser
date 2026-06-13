/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

/* import-globals-from preferences.js */

var gMoreFromMozillaPane = {
  initialized: false,

  renderProducts() {
    let products = [
      {
        id: "doda-hub",
        title_string_id: "more-from-moz-doda-hub-title",
        description_string_id: "more-from-moz-doda-hub-description",
        region: "global",
        button: {
          id: "dodaHub",
          type: "link",
          label_string_id: "more-from-moz-learn-more-link",
          actionURL: "about:doda",
        },
      },
      {
        id: "knowledge-graph",
        title_string_id: "more-from-moz-knowledge-graph-title",
        description_string_id: "more-from-moz-knowledge-graph-description",
        region: "global",
        button: {
          id: "knowledgeGraph",
          type: "link",
          label_string_id: "more-from-moz-learn-more-link",
          actionURL: "about:knowledgegraph",
        },
      },
      {
        id: "ai-assistant",
        title_string_id: "more-from-moz-ai-assistant-title",
        description_string_id: "more-from-moz-ai-assistant-description",
        region: "global",
        button: {
          id: "aiAssistant",
          type: "link",
          label_string_id: "more-from-moz-learn-more-link",
          actionURL: "about:ai",
        },
      },
      {
        id: "trust-os",
        title_string_id: "more-from-moz-trust-os-title",
        description_string_id: "more-from-moz-trust-os-description",
        region: "global",
        button: {
          id: "trustOS",
          type: "link",
          label_string_id: "more-from-moz-learn-more-link",
          actionURL: "about:provenance",
        },
      },
      {
        id: "identity-wallet",
        title_string_id: "more-from-moz-identity-wallet-title",
        description_string_id: "more-from-moz-identity-wallet-description",
        region: "global",
        button: {
          id: "identityWallet",
          type: "link",
          label_string_id: "more-from-moz-learn-more-link",
          actionURL: "about:identity",
        },
      },
      {
        id: "durgashield",
        title_string_id: "more-from-moz-durgashield-title",
        description_string_id: "more-from-moz-durgashield-description",
        region: "global",
        button: {
          id: "durgashield",
          type: "link",
          label_string_id: "more-from-moz-learn-more-link",
          actionURL: "https://dodatech.com/extensions/firefox/durgashield/",
        },
      },
      {
        id: "programmable-browser",
        title_string_id: "more-from-moz-programmable-title",
        description_string_id: "more-from-moz-programmable-description",
        region: "global",
        button: {
          id: "programmableBrowser",
          type: "link",
          label_string_id: "more-from-moz-learn-more-link",
          actionURL: "about:doda",
        },
      },
      {
        id: "tutorials",
        title_string_id: "more-from-moz-tutorials-title",
        description_string_id: "more-from-moz-tutorials-description",
        region: "global",
        button: {
          id: "tutorials",
          type: "link",
          label_string_id: "more-from-moz-learn-more-link",
          actionURL: "https://tutorials.dodatech.com/",
        },
      },
      {
        id: "store",
        title_string_id: "more-from-moz-store-title",
        description_string_id: "more-from-moz-store-description",
        region: "global",
        button: {
          id: "store",
          type: "link",
          label_string_id: "more-from-moz-learn-more-link",
          actionURL: "https://store.dodatech.com/",
        },
      },
    ];

    this._productsContainer = document.getElementById(
      "moreFromDodaCategory"
    );
    let frag = document.createDocumentFragment();
    this._template = document.getElementById("simple");

    if (!this._template) {
      return;
    }

    for (let product of products) {
      let template = this._template.content.cloneNode(true);
      let title = template.querySelector(".product-title");
      let desc = template.querySelector(".description");

      document.l10n.setAttributes(title, product.title_string_id);
      title.id = product.id;

      document.l10n.setAttributes(desc, product.description_string_id);

      let isLink = product.button.type === "link";
      let actionElement = template.querySelector(
        isLink ? ".text-link" : ".small-button"
      );

      if (actionElement) {
        actionElement.hidden = false;
        actionElement.id = product.button.id;
        document.l10n.setAttributes(
          actionElement,
          product.button.label_string_id
        );

        if (isLink) {
          actionElement.setAttribute("href", product.button.actionURL);
        } else {
          actionElement.addEventListener("click", function () {
            let mainWindow = window.windowRoot.window;
            mainWindow.openTrustedLinkIn(
              product.button.actionURL,
              "tab"
            );
          });
        }
      }

      frag.appendChild(template);
    }
    this._productsContainer.appendChild(frag);
  },

  async init() {
    if (this.initialized) {
      return;
    }
    this.initialized = true;
    document
      .getElementById("moreFromDodaCategory")
      .removeAttribute("data-hidden-from-search");
    document
      .getElementById("moreFromDodaCategory-header")
      .removeAttribute("data-hidden-from-search");

    this.renderProducts();
  },
};
