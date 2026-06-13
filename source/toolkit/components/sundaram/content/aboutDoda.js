"use strict";

const DODA = Cc["@mozilla.org/sundaram/service;1"].getService(Ci.nsIDodaService);
const $ = id => document.getElementById(id);

document.addEventListener("keydown", e => {
  if ((e.ctrlKey || e.metaKey) && e.key === "k") {
    e.preventDefault();
    togglePalette();
  }
  if ((e.ctrlKey || e.metaKey) && e.shiftKey && e.key === ",") {
    e.preventDefault();
    togglePalette();
  }
  if (e.key === "Escape" && !$("palette-overlay").classList.contains("hidden")) {
    hidePalette();
  }
});

function togglePalette() {
  const overlay = $("palette-overlay");
  if (overlay.classList.contains("hidden")) {
    showPalette();
  } else {
    hidePalette();
  }
}

function showPalette() {
  $("palette-overlay").classList.remove("hidden");
  $("paletteInput").value = "";
  $("paletteResults").innerHTML = "";
  $("paletteInput").focus();

  try {
    const cmds = JSON.parse(DODA.getCommands());
    $("paletteResults").innerHTML = cmds.map(c =>
      `<div class="palette-item" data-cmd="${c}">${escapeHTML(c)}</div>`
    ).join("");
    document.querySelectorAll(".palette-item").forEach(item => {
      item.addEventListener("click", () => executePaletteCommand(item.dataset.cmd));
    });
  } catch (e) {}
}

function hidePalette() {
  $("palette-overlay").classList.add("hidden");
}

$("paletteInput").addEventListener("input", () => {
  const q = $("paletteInput").value.toLowerCase();
  document.querySelectorAll(".palette-item").forEach(item => {
    item.style.display = item.dataset.cmd.includes(q) ? "block" : "none";
  });
});

$("paletteInput").addEventListener("keydown", e => {
  if (e.key === "Enter") {
    const visible = document.querySelector('.palette-item[style*="display: block"], .palette-item:not([style*="display"])');
    if (visible) {
      executePaletteCommand(visible.dataset.cmd);
    }
  }
});

function executePaletteCommand(cmd) {
  hidePalette();
  const args = prompt(`Execute: ${cmd}\nArguments:`);
  if (args === null) return;
  try {
    const result = DODA.executeCommand(cmd, args || "");
    alert("Result: " + result);
  } catch (e) {
    alert("Error: " + e.message);
  }
}

document.querySelectorAll(".tab").forEach(tab => {
  tab.addEventListener("click", () => {
    document.querySelectorAll(".tab").forEach(t => t.classList.remove("active"));
    document.querySelectorAll(".panel").forEach(p => p.classList.remove("active"));
    tab.classList.add("active");
    const panel = document.getElementById("panel-" + tab.dataset.tab);
    if (panel) panel.classList.add("active");
    if (tab.dataset.tab === "overview") loadOverview();
  });
});

function loadOverview() {
  try {
    const stats = JSON.parse(DODA.getStats());
    $("overviewContent").innerHTML = `
      <div class="stat-row">
        <div class="stat-card">
          <div class="stat-value">${stats.pages_indexed}</div>
          <div class="stat-label">Pages Indexed</div>
        </div>
        <div class="stat-card">
          <div class="stat-value">${stats.claims_extracted}</div>
          <div class="stat-label">Claims Extracted</div>
        </div>
        <div class="stat-card">
          <div class="stat-value">${stats.identities}</div>
          <div class="stat-label">Identities</div>
        </div>
      </div>
      <div class="info-box">
        <strong>Keyboard Shortcut:</strong> Press <kbd>Ctrl+K</kbd> or <kbd>Ctrl+Shift+,</kbd> to open the Command Palette from any page.
      </div>
    `;
  } catch (e) {
    $("overviewContent").innerHTML = `<p class="error">Error: ${e.message}</p>`;
  }
}

$("kgSearchBtn").addEventListener("click", () => {
  const query = $("kgSearchInput").value.trim();
  try {
    const result = DODA.executeCommand("kg:search", query || "*");
    const parsed = JSON.parse(result);
    if (parsed && parsed.length > 0) {
      $("kgResults").innerHTML = parsed.map(p => `
        <div class="card">
          <div class="card-title">${escapeHTML(p.title)}</div>
          <div class="card-url">${escapeHTML(p.url)}</div>
          <div class="card-time">${escapeHTML(p.visited_at)}</div>
        </div>
      `).join("");
    } else {
      $("kgResults").innerHTML = '<p class="empty">No results.</p>';
    }
  } catch (e) {
    $("kgResults").innerHTML = `<p class="error">Error: ${e.message}</p>`;
  }
});

$("kgSearchInput").addEventListener("keydown", e => {
  if (e.key === "Enter") $("kgSearchBtn").click();
});

$("aiChatBtn").addEventListener("click", () => {
  const query = $("aiChatInput").value.trim();
  if (!query) return;
  $("aiResponse").innerHTML = "Thinking...";
  try {
    const result = DODA.executeCommand("ai:chat", query);
    $("aiResponse").innerHTML = `<pre>${escapeHTML(result)}</pre>`;
  } catch (e) {
    $("aiResponse").innerHTML = `<p class="error">Error: ${e.message}</p>`;
  }
});

$("aiChatInput").addEventListener("keydown", e => {
  if (e.key === "Enter") $("aiChatBtn").click();
});

$("pvSearchBtn").addEventListener("click", () => {
  const query = $("pvSearchInput").value.trim();
  try {
    const result = DODA.executeCommand("pv:search", query || "*");
    const parsed = JSON.parse(result);
    if (parsed && parsed.length > 0) {
      $("pvResults").innerHTML = parsed.map(p => `
        <div class="card">
          <div class="card-claim">${escapeHTML(p.claim_text)}</div>
          <div class="card-meta">${escapeHTML(p.domain)} &middot; ${escapeHTML(p.source_url)}</div>
        </div>
      `).join("");
    } else {
      $("pvResults").innerHTML = '<p class="empty">No results.</p>';
    }
  } catch (e) {
    $("pvResults").innerHTML = `<p class="error">Error: ${e.message}</p>`;
  }
});

$("pvSearchInput").addEventListener("keydown", e => {
  if (e.key === "Enter") $("pvSearchBtn").click();
});

function escapeHTML(s) {
  if (!s) return "";
  return String(s).replace(/&/g, "&amp;").replace(/</g, "&lt;").replace(/>/g, "&gt;");
}

loadOverview();
