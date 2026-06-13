/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

// This file contains branding-specific prefs.

pref("startup.homepage_override_url", "");
pref("startup.homepage_welcome_url", "");
pref("startup.homepage_welcome_url.additional", "");
// The time interval between checks for a new version (in seconds)
pref("app.update.interval", 86400); // 24 hours
// Give the user x seconds to react before showing the big UI. default=24 hours
pref("app.update.promptWaitTime", 86400);
// URL user can browse to manually if for some reason all update installation
// attempts fail.
pref("app.support.baseURL", "https://support.dodatech.com/");
pref("app.feedback.baseURL", "https://support.dodatech.com/forum/");
pref("app.update.url.manual", "https://dodatech.com");
pref("app.update.url.details", "https://dodatech.com");

// The number of days a binary is permitted to be old
// without checking for an update.  This assumes that
// app.update.checkInstallTime is true.
pref("app.update.checkInstallTime.days", 2);

// Give the user x seconds to reboot before showing a badge on the hamburger
// button. default=immediately
pref("app.update.badgeWaitTime", 0);

pref("browser.aboutwelcome.enabled", false);
pref("toolkit.telemetry.cachedClientID", "");
pref("toolkit.telemetry.prompted", 2);

// Disable Remote Settings to suppress console spam
pref("services.settings.server", "");
pref("services.settings.loglevel", "Fatal");

// Number of usages of the web console.
// If this is less than 5, then pasting code into the web console is disabled
pref("devtools.selfxss.count", 5);

// === TELEMETRY & DATA COLLECTION ===
pref("datareporting.healthreport.uploadEnabled", false);
pref("datareporting.policy.dataSubmissionEnabled", false);
pref("toolkit.telemetry.enabled", false);
pref("toolkit.telemetry.archive.enabled", false);
pref("toolkit.telemetry.bhrPing.enabled", false);
pref("toolkit.telemetry.firstShutdownPing.enabled", false);
pref("toolkit.telemetry.hybridContent.enabled", false);
pref("toolkit.telemetry.newProfilePing.enabled", false);
pref("toolkit.telemetry.rejected", true);
pref("toolkit.telemetry.server", "data:,");
pref("toolkit.telemetry.shutdownPingSender.enabled", false);
pref("toolkit.telemetry.unified", false);
pref("toolkit.telemetry.updatePing.enabled", false);
pref("browser.newtabpage.activity-stream.feeds.telemetry", false);
pref("browser.newtabpage.activity-stream.telemetry", false);
pref("browser.newtabpage.activity-stream.feeds.system.topstories", false);
pref("browser.newtabpage.activity-stream.feeds.section.topstories", false);
pref("browser.newtabpage.activity-stream.feeds.discoverystreamfeed", false);
pref("browser.newtabpage.activity-stream.showSponsoredCheckboxes", false);
pref("devtools.onboarding.telemetry.logged", false);
pref("browser.ping-centre.telemetry", false);

// === PRIVACY ===
// Use FPP (Fingerprinting Protection) instead of RFP (Resist Fingerprinting)
// FPP is more compatible with modern websites and uses the ETP allow list
// for exemptions, which hooks into our ContentBlockingAllowList C++ bypass
// for dodatech.com.
pref("privacy.resistFingerprinting", false);
pref("privacy.fingerprintingProtection", true);
pref("privacy.fingerprintingProtection.pbmode", true);
pref("privacy.trackingprotection.enabled", true);
pref("privacy.trackingprotection.pbmode.enabled", true);
pref("privacy.trackingprotection.socialtracking.enabled", true);
pref("privacy.trackingprotection.cryptomining.enabled", true);
pref("privacy.trackingprotection.fingerprinting.enabled", true);
pref("privacy.partition.network_state", true);
pref("privacy.partition.always_partition_third_party_non_cookie_storage", true);
pref("privacy.firstparty.isolate", true);
pref("privacy.window.name.update.enabled", false);
pref("network.cookie.cookieBehavior", 1);
pref("network.cookie.thirdparty.sessionOnly", true);
pref("network.cookie.lifetimePolicy", 2);
pref("network.cookie.lifetime.days", 0);
pref("browser.privatebrowsing.autostart", false);
pref("signon.rememberSignons", false);
pref("signon.autofillForms", false);
pref("signon.formlessCapture.enabled", false);
pref("dom.storage.snapshot_reusing", false);
pref("browser.formfill.enable", false);

// === WEBRTC / MEDIA ===
pref("media.peerconnection.enabled", false);
pref("media.navigator.enabled", false);
pref("media.getusermedia.screensharing.enabled", false);
pref("media.video_stats.enabled", false);

// === GEOLOCATION ===
pref("geo.enabled", false);
pref("browser.search.geoip.url", "");
pref("browser.search.geoSpecificDefaults", false);
pref("permissions.default.geo", 2);

// === SAFE BROWSING ===
pref("browser.safebrowsing.enabled", false);
pref("browser.safebrowsing.malware.enabled", false);
pref("browser.safebrowsing.phishing.enabled", false);
pref("browser.safebrowsing.downloads.enabled", false);
pref("browser.safebrowsing.downloads.remote.enabled", false);
pref("browser.safebrowsing.downloads.remote.url", "");
pref("browser.safebrowsing.downloads.remote.block_potentially_unwanted", false);
pref("browser.safebrowsing.downloads.remote.block_uncommon", false);

// === POCKET ===
pref("extensions.pocket.enabled", false);
pref("extensions.pocket.api", "");
pref("extensions.pocket.oAuthConsumerKey", "");
pref("extensions.pocket.site", "");

// === SYNC ===
pref("services.sync.engine.addons", false);
pref("services.sync.engine.bookmarks", false);
pref("services.sync.engine.history", false);
pref("services.sync.engine.passwords", false);
pref("services.sync.engine.prefs", false);
pref("services.sync.engine.tabs", false);
pref("services.sync.engine.addresses", false);
pref("services.sync.engine.creditcards", false);
pref("services.sync.enabled", false);

// === STUDIES / EXPERIMENTS ===
pref("app.normandy.enabled", false);
pref("app.shield.optoutstudies.enabled", false);
pref("browser.discovery.enabled", false);
pref("network.allow-experiments", false);
pref("experiments.enabled", false);
pref("experiments.supported", false);
pref("experiments.manifest.uri", "");
pref("nimbus.rollouts.enabled", false);
pref("nimbus.validation.enabled", false);
pref("messaging-system.rsexperimentloader.collection_id", "");

// === UPDATES ===
pref("app.update.enabled", false);
pref("app.update.auto", false);
pref("app.update.url", "");

// === DNS / NETWORK ===
pref("network.dns.disablePrefetch", true);
pref("network.prefetch-next", false);
pref("network.trr.mode", 3);
pref("network.trr.uri", "https://mozilla.cloudflare-dns.com/dns-query");
pref("network.trr.custom_uri", "");
pref("network.http.referer.XOriginTrimmingPolicy", 2);
pref("network.http.referer.spoofSource", true);
pref("network.IDN_show_punycode", true);

// === HTTPS ===
pref("dom.security.https_only_mode", true);
pref("dom.security.https_only_mode_ever_enabled", true);
pref("dom.security.https_only_mode_error_page_user_suggestions", false);

// === UI ===
pref("browser.uidensity", 1);
pref("browser.tabs.allow_transparent_browser", false);
pref("browser.tabs.drawInTitlebar", true);
pref("browser.tabs.insertAfterCurrent", false);
pref("browser.tabs.loadInBackground", false);
pref("browser.urlbar.placeholderName", "DuckDuckGo");
pref("browser.urlbar.placeholderName.private", "DuckDuckGo");
