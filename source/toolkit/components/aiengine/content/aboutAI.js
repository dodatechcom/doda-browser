"use strict";

const AI = Cc["@mozilla.org/aiengine/service;1"].getService(Ci.nsIAIEngineService);
const $ = id => document.getElementById(id);

function updateStatus() {
  try {
    const ok = AI.checkHealth();
    $("statusBar").innerHTML = ok
      ? '<span class="status-ok">&#9679; Ollama connected</span>'
      : '<span class="status-warn">&#9679; Ollama not reachable (start it: ollama serve)</span>';
  } catch (e) {
    $("statusBar").innerHTML = `<span class="status-warn">&#9679; Error: ${e.message}</span>`;
  }
}

document.querySelectorAll(".tab").forEach(tab => {
  tab.addEventListener("click", () => {
    document.querySelectorAll(".tab").forEach(t => t.classList.remove("active"));
    document.querySelectorAll(".panel").forEach(p => p.classList.remove("active"));
    tab.classList.add("active");
    document.getElementById("panel-" + tab.dataset.tab).classList.add("active");
  });
});

$("chatSendBtn").addEventListener("click", async () => {
  const input = $("chatInput");
  const msg = input.value.trim();
  if (!msg) return;

  const msgs = $("chatMessages");
  msgs.innerHTML += `<div class="msg user">${escapeHTML(msg)}</div>`;
  input.value = "";

  const history = AI.getHistory(50);
  let historyJson = "";
  try {
    const parsed = JSON.parse(history);
    if (parsed && parsed.length > 0) {
      historyJson = parsed.map(h => `{"role":"${escapeJSON(h.role)}","content":"${escapeJSON(h.content)}"}`).reverse().join(",");
    }
  } catch (e) {}

  try {
    const response = AI.chat(msg, historyJson);
    msgs.innerHTML += `<div class="msg assistant">${escapeHTML(response)}</div>`;
  } catch (e) {
    msgs.innerHTML += `<div class="msg error">Error: ${escapeHTML(e.message)}</div>`;
  }
  msgs.scrollTop = msgs.scrollHeight;
});

$("chatInput").addEventListener("keydown", e => {
  if (e.key === "Enter" && !e.shiftKey) {
    e.preventDefault();
    $("chatSendBtn").click();
  }
});

$("tabAnalyzeBtn").addEventListener("click", async () => {
  const prompt = $("tabPrompt").value.trim() || "Summarize this page.";
  $("tabResult").innerHTML = "Analyzing...";
  try {
    const response = AI.analyzeTab("", prompt);
    $("tabResult").innerHTML = `<pre>${escapeHTML(response)}</pre>`;
  } catch (e) {
    $("tabResult").innerHTML = `<p class="error">Error: ${escapeHTML(e.message)}</p>`;
  }
});

$("crossAnalyzeBtn").addEventListener("click", async () => {
  const pages = $("crossPages").value.trim();
  const prompt = $("crossPrompt").value.trim() || "Compare these pages.";
  if (!pages) return;

  $("crossResult").innerHTML = "Analyzing...";
  try {
    const response = AI.crossPage(pages, prompt);
    $("crossResult").innerHTML = `<pre>${escapeHTML(response)}</pre>`;
  } catch (e) {
    $("crossResult").innerHTML = `<p class="error">Error: ${escapeHTML(e.message)}</p>`;
  }
});

$("debateStartBtn").addEventListener("click", async () => {
  const topic = $("debateTopic").value.trim();
  const perspectives = $("debatePerspectives").value.trim();
  if (!topic) return;

  $("debateResult").innerHTML = "Debating...";
  try {
    const response = AI.debate(topic, perspectives || "pro, con, neutral");
    $("debateResult").innerHTML = `<pre>${escapeHTML(response)}</pre>`;
  } catch (e) {
    $("debateResult").innerHTML = `<p class="error">Error: ${escapeHTML(e.message)}</p>`;
  }
});

function escapeHTML(s) {
  if (!s) return "";
  return String(s).replace(/&/g, "&amp;").replace(/</g, "&lt;").replace(/>/g, "&gt;");
}

function escapeJSON(s) {
  if (!s) return "";
  return String(s).replace(/\\/g, "\\\\").replace(/"/g, "\\\"").replace(/\n/g, "\\n").replace(/\r/g, "\\r").replace(/\t/g, "\\t");
}

updateStatus();
