/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

import { Preferences } from "chrome://global/content/preferences/Preferences.mjs";
import { SettingGroupManager } from "chrome://browser/content/preferences/config/SettingGroupManager.mjs";

const { AppConstants } = ChromeUtils.importESModule(
  "resource://gre/modules/AppConstants.sys.mjs"
);

Preferences.addSetting({
  id: "updateAppInfo",
  getControlConfig(config) {
    let version = AppConstants.MOZ_APP_VERSION_DISPLAY;
    let distribution;
    let distributionId;

    // Include the build ID if this is an "a#" (nightly) build
    if (/a\d+$/.test(version)) {
      let buildID = Services.appinfo.appBuildID;
      let year = buildID.slice(0, 4);
      let month = buildID.slice(4, 6);
      let day = buildID.slice(6, 8);
      version += ` (${year}-${month}-${day})`;
    }

    // Append "(32-bit)" or "(64-bit)" build architecture to the version number:
    let bundle = Services.strings.createBundle(
      "chrome://browser/locale/browser.properties"
    );
    let archResource = Services.appinfo.is64Bit
      ? "aboutDialog.architecture.sixtyFourBit"
      : "aboutDialog.architecture.thirtyTwoBit";
    let arch = bundle.GetStringFromName(archResource);
    version += ` (${arch})`;

    let defaults = Services.prefs.getDefaultBranch(null);
    let distroId = defaults.getCharPref("distribution.id", "");
    if (distroId) {
      let distroString = distroId;

      let distroVersion = defaults.getCharPref("distribution.version", "");
      if (distroVersion) {
        distroString += " - " + distroVersion;
      }

      distributionId = distroString;

      let distroAbout = defaults.getStringPref("distribution.about", "");
      distribution = distroAbout;
    }

    config.controlAttrs = {
      ".version": version,
      ".distribution": distribution,
      ".distributionId": distributionId,
    };

    return config;
  },
});

// Support links
Preferences.addSetting({
  id: "supportLinksGroup",
});
Preferences.addSetting({
  id: "supportGetHelp",
});
Preferences.addSetting({
  id: "supportShareIdeas",
});

SettingGroupManager.registerGroups({
  about: {
    inProgress: true,
    l10nId: "about-application-heading",
    iconSrc: "chrome://browser/skin/sidebar/firefox.svg",
    headingLevel: 2,
    items: [
      {
        id: "updateAppInfo",
        control: "update-information",
      },
      {
        id: "getDownload",
        control: "moz-box-link",
        l10nId: "about-application-get-latest",
        controlAttrs: {
          href: "https://github.com/dodatech/browser/releases/latest",
        },
      },
    ],
  },
  support: {
    inProgress: true,
    l10nId: "support-application-heading",
    iconSrc: "chrome://global/skin/icons/help.svg",
    headingLevel: 2,
    items: [
      {
        id: "supportLinksGroup",
        control: "moz-box-group",
        items: [
          {
            id: "supportGetHelp",
            l10nId: "support-get-help",
            control: "moz-box-link",
            controlAttrs: {
              href: "https://support.dodatech.com/",
            },
          },
          {
            id: "supportShareIdeas",
            l10nId: "support-share-ideas",
            control: "moz-box-link",
            controlAttrs: {
              href: "https://support.dodatech.com/forum/",
            },
          },
        ],
      },
    ],
  },
});
