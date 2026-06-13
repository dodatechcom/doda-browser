"use strict";

const PV = Cc["@mozilla.org/provenance/service;1"].getService(Ci.nsIProvenanceService);
const $ = id => document.getElementById(id);

document.querySelectorAll(".tab").forEach(tab => {
  tab.addEventListener("click", () => {
    document.querySelectorAll(".tab").forEach(t => t.classList.remove("active"));
    document.querySelectorAll(".panel").forEach(p => p.classList.remove("active"));
    tab.classList.add("active");
    document.getElementById("panel-" + tab.dataset.tab).classList.add("active");
    if (tab.dataset.tab === "stats") loadStats();
  });
});

$("claimSearchBtn").addEventListener("click", () => {
  const query = $("claimSearchInput").value.trim();
  searchClaims(query);
});

$("claimSearchInput").addEventListener("keydown", e => {
  if (e.key === "Enter") {
    $("claimSearchBtn").click();
  }
});

function searchClaims(query) {
  try {
    let results;
    if (query) {
      results = JSON.parse(PV.searchClaims(query, 50));
    } else {
      results = JSON.parse(PV.getRecent(50));
    }
    renderClaims(results);
  } catch (e) {
    $("claimList").innerHTML = `<p class="error">Error: ${e.message}</p>`;
  }
}

function renderClaims(claims) {
  if (!claims || claims.length === 0) {
    $("claimList").innerHTML = '<p class="empty">No claims found.</p>';
    return;
  }
  $("claimList").innerHTML = claims.map(c => `
    <div class="card">
      <div class="card-claim">${escapeHTML(c.claim_text)}</div>
      <div class="card-meta">
        Source: <a href="${escapeHTML(c.source_url)}" target="_blank">${escapeHTML(c.source_title || c.source_url)}</a>
        &middot; Domain: ${escapeHTML(c.domain)} &middot; Confidence: ${(c.confidence * 100).toFixed(0)}%
      </div>
      <div class="card-id">ID: ${escapeHTML(c.id)}</div>
    </div>
  `).join("");
}

$("compareBtn").addEventListener("click", () => {
  const claimId = $("compareClaimId").value.trim();
  if (!claimId) return;

  try {
    const results = JSON.parse(PV.compareSources(claimId));
    if (!results || results.length === 0) {
      $("compareResult").innerHTML = '<p class="empty">No sources found for this claim.</p>';
      return;
    }
    $("compareResult").innerHTML = results.map(r => `
      <div class="card">
        <div class="card-claim">${escapeHTML(r.claim_text)}</div>
        <div class="card-meta">
          Source: <a href="${escapeHTML(r.source_url)}" target="_blank">${escapeHTML(r.source_title || r.source_url)}</a>
          &middot; Domain: ${escapeHTML(r.domain)} &middot; Confidence: ${(r.confidence * 100).toFixed(0)}%
        </div>
      </div>
    `).join("");
  } catch (e) {
    $("compareResult").innerHTML = `<p class="error">Error: ${e.message}</p>`;
  }
});

$("spreadBtn").addEventListener("click", () => {
  const claimId = $("spreadClaimId").value.trim();
  if (!claimId) return;

  try {
    const results = JSON.parse(PV.findSpread(claimId, 50));
    if (!results || results.length === 0) {
      $("spreadResult").innerHTML = '<p class="empty">No spread detected for this claim.</p>';
      return;
    }
    $("spreadResult").innerHTML = results.map(r => `
      <div class="card">
        <div class="card-claim">${escapeHTML(r.claim_text)}</div>
        <div class="card-meta">
          Source: <a href="${escapeHTML(r.source_url)}" target="_blank">${escapeHTML(r.source_title || r.source_url)}</a>
          &middot; Domain: ${escapeHTML(r.domain)}
        </div>
      </div>
    `).join("");
  } catch (e) {
    $("spreadResult").innerHTML = `<p class="error">Error: ${e.message}</p>`;
  }
});

function loadStats() {
  try {
    const count = PV.getClaimCount();
    $("statsContent").innerHTML = `
      <div class="stat-card">
        <div class="stat-value">${count}</div>
        <div class="stat-label">Total Claims Extracted</div>
      </div>
      <div class="info-box">
        Trust OS extracts factual claims from pages you visit, tracks them across sources,
        and helps you understand how information spreads across the web.
      </div>
    `;
  } catch (e) {
    $("statsContent").innerHTML = `<p class="error">Error: ${e.message}</p>`;
  }
}

function escapeHTML(s) {
  if (!s) return "";
  return String(s).replace(/&/g, "&amp;").replace(/</g, "&lt;").replace(/>/g, "&gt;");
}

searchClaims("");
