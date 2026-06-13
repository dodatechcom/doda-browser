"use strict";

const ID = Cc["@mozilla.org/identitywallet/service;1"].getService(Ci.nsIIdentityWalletService);
const $ = id => document.getElementById(id);

document.querySelectorAll(".tab").forEach(tab => {
  tab.addEventListener("click", () => {
    document.querySelectorAll(".tab").forEach(t => t.classList.remove("active"));
    document.querySelectorAll(".panel").forEach(p => p.classList.remove("active"));
    tab.classList.add("active");
    const panel = document.getElementById("panel-" + tab.dataset.tab);
    if (panel) panel.classList.add("active");
    if (tab.dataset.tab === "dashboard") loadDashboard();
    if (tab.dataset.tab === "identities") loadIdentities();
    if (tab.dataset.tab === "permissions") loadPermissions();
  });
});

function loadDashboard() {
  try {
    const identities = JSON.parse(ID.listIdentities());
    const permissions = JSON.parse(ID.listSitePermissions());
    $("dashboardContent").innerHTML = `
      <div style="margin-bottom: 16px;">
        <div class="stat-card">
          <div class="stat-value">${identities.length}</div>
          <div class="stat-label">Identities</div>
        </div>
        <div class="stat-card">
          <div class="stat-value">${permissions.length}</div>
          <div class="stat-label">Site Permissions</div>
        </div>
      </div>
      <div class="info-box">
        Your cryptographic keys never leave this device. Log into sites without passwords.
      </div>
    `;
  } catch (e) {
    $("dashboardContent").innerHTML = `<p class="error">Error: ${e.message}</p>`;
  }
}

function loadIdentities() {
  try {
    const ids = JSON.parse(ID.listIdentities());
    if (!ids || ids.length === 0) {
      $("identityList").innerHTML = '<p class="empty">No identities yet. Create one above.</p>';
      return;
    }
    $("identityList").innerHTML = ids.map(id => `
      <div class="card">
        <div class="card-title">${escapeHTML(id.name || "Unnamed")}</div>
        <div class="card-sub">${escapeHTML(id.did)}</div>
        <div class="card-meta">Created: ${escapeHTML(id.created_at)}</div>
        <button class="btn-danger" data-did="${escapeHTML(id.did)}">Delete</button>
      </div>
    `).join("");

    document.querySelectorAll("[data-did]").forEach(btn => {
      btn.addEventListener("click", () => {
        if (confirm("Delete this identity?")) {
          ID.deleteIdentity(btn.dataset.did);
          loadIdentities();
        }
      });
    });
  } catch (e) {
    $("identityList").innerHTML = `<p class="error">Error: ${e.message}</p>`;
  }
}

$("createIdentityBtn").addEventListener("click", () => {
  const name = $("identityName").value.trim() || "Default";
  try {
    ID.createIdentity(name);
    $("identityName").value = "";
    loadIdentities();
  } catch (e) {
    alert("Error: " + e.message);
  }
});

function loadPermissions() {
  try {
    const perms = JSON.parse(ID.listSitePermissions());
    if (!perms || perms.length === 0) {
      $("permissionList").innerHTML = '<p class="empty">No site permissions granted yet.</p>';
      return;
    }
    $("permissionList").innerHTML = perms.map(p => `
      <div class="card">
        <div class="card-title">${escapeHTML(p.site)}</div>
        <div class="card-sub">Identity: ${escapeHTML(p.did)}</div>
        <div class="card-meta">Granted: ${escapeHTML(p.granted_at)}</div>
        <button class="btn-danger" data-revoke="${escapeHTML(p.did)}" data-site="${escapeHTML(p.site)}">Revoke</button>
      </div>
    `).join("");

    document.querySelectorAll("[data-revoke]").forEach(btn => {
      btn.addEventListener("click", () => {
        ID.revokeSitePermission(btn.dataset.revoke, btn.dataset.site);
        loadPermissions();
      });
    });
  } catch (e) {
    $("permissionList").innerHTML = `<p class="error">Error: ${e.message}</p>`;
  }
}

function escapeHTML(s) {
  if (!s) return "";
  return String(s).replace(/&/g, "&amp;").replace(/</g, "&lt;").replace(/>/g, "&gt;").replace(/"/g, "&quot;");
}

loadDashboard();
