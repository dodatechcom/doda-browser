/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import { Preferences } from "chrome://global/content/preferences/Preferences.mjs";
import { SettingGroupManager } from "chrome://browser/content/preferences/config/SettingGroupManager.mjs";

/**
 * Builds the list of Doda features to display.
 *
 * @returns {object[]}
 */
function getProducts() {
  return [
    {
      id: "doda-hub",
      l10nId: "more-from-moz-doda-hub-card",
      region: "global",
      link: {
        l10nId: "more-from-moz-doda-hub-box-link",
        iconSrc: "chrome://global/skin/icons/highlights.svg",
        actionURL: "about:doda",
      },
    },
    {
      id: "knowledge-graph",
      l10nId: "more-from-moz-knowledge-graph-card",
      region: "global",
      link: {
        l10nId: "more-from-moz-knowledge-graph-box-link",
        iconSrc: "chrome://global/skin/icons/developer.svg",
        actionURL: "about:knowledgegraph",
      },
    },
    {
      id: "ai-assistant",
      l10nId: "more-from-moz-ai-assistant-card",
      region: "global",
      link: {
        l10nId: "more-from-moz-ai-assistant-box-link",
        iconSrc: "chrome://global/skin/icons/highlights.svg",
        actionURL: "about:ai",
      },
    },
    {
      id: "trust-os",
      l10nId: "more-from-moz-trust-os-card",
      region: "global",
      link: {
        l10nId: "more-from-moz-trust-os-box-link",
        iconSrc: "chrome://global/skin/icons/security.svg",
        actionURL: "about:provenance",
      },
    },
    {
      id: "identity-wallet",
      l10nId: "more-from-moz-identity-wallet-card",
      region: "global",
      link: {
        l10nId: "more-from-moz-identity-wallet-box-link",
        iconSrc: "chrome://browser/skin/fingerprint.svg",
        actionURL: "about:identity",
      },
    },
    {
      id: "durgashield",
      l10nId: "more-from-moz-durgashield-card",
      region: "global",
      link: {
        l10nId: "more-from-moz-durgashield-box-link",
        iconSrc: "chrome://global/skin/icons/security.svg",
        actionURL: "https://dodatech.com/extensions/firefox/durgashield/",
      },
    },
    {
      id: "programmable-browser",
      l10nId: "more-from-moz-programmable-card",
      region: "global",
      link: {
        l10nId: "more-from-moz-programmable-box-link",
        iconSrc: "chrome://global/skin/icons/developer.svg",
        actionURL: "about:doda",
      },
    },
    {
      id: "tutorials",
      l10nId: "more-from-moz-tutorials-card",
      region: "global",
      link: {
        l10nId: "more-from-moz-tutorials-box-link",
        iconSrc: "chrome://global/skin/icons/info.svg",
        actionURL: "https://tutorials.dodatech.com/",
      },
    },
    {
      id: "store",
      l10nId: "more-from-moz-store-card",
      region: "global",
      link: {
        l10nId: "more-from-moz-store-box-link",
        iconSrc: "chrome://browser/skin/places/tag.svg",
        actionURL: "https://store.dodatech.com/",
      },
    },
    {
      id: "doda-new-products",
      l10nId: "more-from-moz-new-products-card2",
      region: "global",
      link: {
        l10nId: "more-from-moz-new-products-box-link",
        iconSrc: "chrome://browser/skin/preferences/mozilla-16.svg",
        actionURL: "https://dodatech.com/",
      },
    },
  ];
}

Preferences.addSetting({
  id: "moreFromDodaProductGrid",
});

SettingGroupManager.registerGroups({
  moreFromDodaProducts: {
    card: "never",
    items: [
      {
        id: "moreFromDodaProductGrid",
        control: "div",
        controlAttrs: {
          class: "products-grid",
        },
        options: getProducts().map(product => ({
          control: "moz-card",
          controlAttrs: {
            id: product.id,
          },
          options: [
            {
              control: "moz-fieldset",
              l10nId: product.l10nId,
              controlAttrs: {
                headinglevel: 2,
              },
            },
            {
              l10nId: product.link.l10nId,
              control: "moz-box-link",
              controlAttrs: {
                iconsrc: product.link.iconSrc || "",
                layout: "large-icon",
                href: product.link.actionURL,
              },
            },
          ],
        })),
      },
    ],
  },
});
