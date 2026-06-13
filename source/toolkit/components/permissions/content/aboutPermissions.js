"use strict";

const PERM_NAMES = {
  "camera": "Camera",
  "microphone": "Microphone",
  "geo": "Location",
  "desktop-notification": "Notifications",
  "persistent-storage": "Persistent Storage",
  "storage-access": "Storage Access",
  "autoplay-media": "Autoplay",
  "midi": "MIDI",
  "midi-sysex": "MIDI Sysex",
  "pointer-lock": "Pointer Lock",
  "vr": "WebXR (VR)",
  "screen": "Screen Capture",
  "3rdparty-storage": "Third-Party Storage",
  "captured-surface-control": "Tab Capture Control",
  "identity-credentials-get": "Identity Credentials",
};

const CAP_NAMES = {
  1: "Allow",
  2: "Deny",
  3: "Prompt",
};

const CAP_CLASS = {
  1: "allow",
  2: "deny",
  3: "prompt",
};

function formatOrigin(origin) {
  try {
    let u = new URL(origin);
    return u.hostname || origin;
  } catch {
    return origin;
  }
}

function refreshList() {
  let content = document.getElementById("content");
  let summary = document.getElementById("summary");

  let allPerms = Services.perms.all;
  let groups = new Map();

  for (let perm of allPerms) {
    let origin;
    try {
      origin = perm.principal.origin;
    } catch {
      origin = perm.principal.URI?.spec || "(unknown)";
    }

    if (!groups.has(origin)) {
      groups.set(origin, []);
    }
    groups.get(origin).push(perm);
  }

  let totalPerms = allPerms.length;
  let totalSites = groups.size;
  summary.textContent = `${totalSites} site${totalSites !== 1 ? "s" : ""} \u00b7 ${totalPerms} permission${totalPerms !== 1 ? "s" : ""}`;

  if (totalPerms === 0) {
    content.innerHTML = `<div class="empty-state"><h3>No custom permissions set</h3><p>Permissions will appear here once websites request them.</p></div>`;
    return;
  }

  let html = "";
  for (let [origin, perms] of groups) {
    let displayOrigin = formatOrigin(origin);
    let favicon = `https://icons.duckduckgo.com/ip3/${new URL(origin).hostname}.ico`;

    html += `<div class="site-group">`;
    html += `<div class="site-header">`;
    html += `<img class="favicon" src="${favicon}" alt="" onerror="this.style.display='none'">`;
    html += `<span>${displayOrigin}</span>`;
    html += `<span class="count">${perms.length} permission${perms.length !== 1 ? "s" : ""}</span>`;
    html += `</div>`;

    for (let perm of perms) {
      let typeName = PERM_NAMES[perm.type] || perm.type;
      let capName = CAP_NAMES[perm.capability] || perm.capability;
      let capClass = CAP_CLASS[perm.capability] || "";

      let expiry = "";
      if (perm.expireType === Services.perms.EXPIRE_SESSION) {
        expiry = "(session)";
      } else if (perm.expireType === Services.perms.EXPIRE_TIME) {
        let d = new Date(perm.expireTime / 1000);
        expiry = `(expires ${d.toLocaleDateString()})`;
      }

      html += `<div class="perm-row" data-origin="${origin}" data-type="${perm.type}">`;
      html += `<span class="perm-type">${typeName}</span>`;
      html += `<span class="perm-value ${capClass}">${capName}</span>`;
      if (expiry) {
        html += `<span class="perm-expiry">${expiry}</span>`;
      }
      html += `<button class="btn-revoke" onclick="revokePermission(this)">Revoke</button>`;
      html += `</div>`;
    }
    html += `</div>`;
  }
  content.innerHTML = html;
}

function revokePermission(btn) {
  let row = btn.closest(".perm-row");
  let origin = row.dataset.origin;
  let type = row.dataset.type;

  let allPerms = Services.perms.all;
  for (let perm of allPerms) {
    let permOrigin;
    try {
      permOrigin = perm.principal.origin;
    } catch {
      continue;
    }
    if (permOrigin === origin && perm.type === type) {
      Services.perms.removePermission(perm);
      break;
    }
  }

  refreshList();
}

document.addEventListener("DOMContentLoaded", refreshList);
