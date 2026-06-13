"use strict";

const KG = Cc["@mozilla.org/knowledgegraph/service;1"].getService(Ci.nsIKnowledgeGraphService);

const $ = id => document.getElementById(id);

document.querySelectorAll(".tab").forEach(tab => {
  tab.addEventListener("click", () => {
    document.querySelectorAll(".tab").forEach(t => t.classList.remove("active"));
    document.querySelectorAll(".panel").forEach(p => p.classList.remove("active"));
    tab.classList.add("active");
    const panel = document.getElementById("panel-" + tab.dataset.tab);
    if (panel) panel.classList.add("active");
    if (tab.dataset.tab === "recent") loadRecent();
    if (tab.dataset.tab === "stats") loadStats();
  });
});

$("searchBtn").addEventListener("click", () => doSearch());
$("searchInput").addEventListener("keydown", e => {
  if (e.key === "Enter") doSearch();
});

function doSearch() {
  const query = $("searchInput").value.trim();
  if (!query) return;
  $("searchResults").innerHTML = "<p class='loading'>Searching...</p>";
  try {
    const result = KG.search(query, 50);
    renderResults($("searchResults"), JSON.parse(result), query);
  } catch (e) {
    $("searchResults").innerHTML = `<p class="error">Search error: ${e.message}</p>`;
  }
}

function renderResults(container, results, query) {
  if (!results || results.length === 0) {
    container.innerHTML = "<p class='empty'>No results found.</p>";
    return;
  }
  container.innerHTML = results.map(p => `
    <div class="result-card">
      <div class="result-title"><a href="${escapeHTML(p.url)}" target="_blank">${escapeHTML(p.title || p.url)}</a></div>
      <div class="result-url">${escapeHTML(p.url)}</div>
      <div class="result-meta">${escapeHTML(p.domain)} &middot; ${escapeHTML(p.visited_at || "unknown date")}</div>
      <div class="result-snippet">${snippet(p.text_content, query)}</div>
    </div>
  `).join("");
}

function snippet(text, query) {
  if (!text) return "";
  const idx = text.toLowerCase().indexOf(query.toLowerCase());
  if (idx === -1) return escapeHTML(text.slice(0, 200));
  const start = Math.max(0, idx - 60);
  const end = Math.min(text.length, idx + 140);
  return (start > 0 ? "..." : "") + escapeHTML(text.slice(start, end)) + (end < text.length ? "..." : "");
}

async function loadRecent() {
  $("recentList").innerHTML = "<p class='loading'>Loading...</p>";
  try {
    const result = KG.getRecent(100);
    const pages = JSON.parse(result);
    if (!pages || pages.length === 0) {
      $("recentList").innerHTML = "<p class='empty'>No pages indexed yet. Browse the web to populate the graph.</p>";
      return;
    }
    $("recentList").innerHTML = pages.map(p => `
      <div class="result-card">
        <div class="result-title"><a href="${escapeHTML(p.url)}" target="_blank">${escapeHTML(p.title || p.url)}</a></div>
        <div class="result-url">${escapeHTML(p.url)}</div>
        <div class="result-meta">${escapeHTML(p.domain)} &middot; ${escapeHTML(p.visited_at || "unknown date")}</div>
      </div>
    `).join("");
  } catch (e) {
    $("recentList").innerHTML = `<p class="error">Error: ${e.message}</p>`;
  }
}

function loadStats() {
  try {
    const count = KG.getPageCount();
    $("statPageCount").textContent = count;
    $("pageCount").textContent = count;
  } catch (e) {
    $("statPageCount").textContent = "Error";
  }
}

$("clearBtn").addEventListener("click", () => {
  if (confirm("Clear all Knowledge Graph data? This cannot be undone.")) {
    try {
      KG.clear();
      loadStats();
      $("recentList").innerHTML = "<p class='empty'>All data cleared.</p>";
      $("searchResults").innerHTML = "";
    } catch (e) {
      alert("Error: " + e.message);
    }
  }
});

function escapeHTML(s) {
  if (!s) return "";
  return String(s).replace(/&/g, "&amp;").replace(/</g, "&lt;").replace(/>/g, "&gt;").replace(/"/g, "&quot;");
}

try {
  const count = KG.getPageCount();
  $("pageCount").textContent = count;
} catch (e) {
  $("pageCount").textContent = "?";
}
