/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

"use strict";

var { AppConstants } = ChromeUtils.importESModule(
  "resource://gre/modules/AppConstants.sys.mjs"
);

function init() {
  let parentWindow = Services.wm.getMostRecentBrowserWindow();
  if (parentWindow) {
    parentWindow.addEventListener("unload", () => {
      if (!window.closed) {
        window.close();
      }
    }, { once: true });
  }

  let versionIdMap = new Map([
    ["base", "aboutDialog-version"],
    ["base-nightly", "aboutDialog-version-nightly"],
    ["base-arch", "aboutdialog-version-arch"],
    ["base-arch-nightly", "aboutdialog-version-arch-nightly"],
  ]);
  let versionIdKey = "base";
  let versionAttributes = {
    version: AppConstants.MOZ_APP_VERSION_DISPLAY,
  };

  let arch = Services.sysinfo.get("arch");
  if (["x86", "x86-64"].includes(arch)) {
    versionAttributes.bits = Services.appinfo.is64Bit ? 64 : 32;
  } else {
    versionIdKey += "-arch";
    versionAttributes.arch = arch;
  }

  let version = Services.appinfo.version;
  if (/a\d+$/.test(version)) {
    versionIdKey += "-nightly";
    let buildID = Services.appinfo.appBuildID;
    let year = buildID.slice(0, 4);
    let month = buildID.slice(4, 6);
    let day = buildID.slice(6, 8);
    versionAttributes.isodate = `${year}-${month}-${day}`;
  }

  let versionField = document.getElementById("version");
  document.l10n.setAttributes(
    versionField,
    versionIdMap.get(versionIdKey),
    versionAttributes
  );

  document
    .getElementById("aboutDialogEscapeKey")
    .addEventListener("command", () => {
      window.close();
    });
}

init();
