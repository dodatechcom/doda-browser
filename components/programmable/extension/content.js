// Doda Programmable Browser — content script

let recording = false;
let recordedSteps = [];
let playbackActive = false;

// === RECORDING ===
function handleEvent(e) {
  if (!recording) return;

  // Don't record our own injected UI
  if (e.target.closest("#doda-recorder-bar")) return;

  const target = e.target;
  const step = {
    type: e.type,
    selector: buildSelector(target),
    tagName: target.tagName?.toLowerCase() || "",
    value: target.value !== undefined ? String(target.value).slice(0, 200) : null,
    text: (target.textContent || "").trim().slice(0, 100) || null,
    href: target.href || null,
    x: e.clientX != null ? e.clientX : null,
    y: e.clientY != null ? e.clientY : null,
    url: location.href,
    timestamp: Date.now(),
  };

  // Debounce input events — only capture on blur/change
  if (e.type === "input") return;

  recordedSteps.push(step);
}

// Reuse the selector logic from background
function buildSelector(el) {
  if (!el || el === document) return "html";
  if (el.id) return `#${CSS.escape(el.id)}`;
  const path = [];
  let current = el;
  while (current && current !== document.body && current !== document.documentElement) {
    let selector = current.tagName.toLowerCase();
    if (current.id) {
      path.unshift(`#${CSS.escape(current.id)}`);
      break;
    }
    if (current.className && typeof current.className === "string") {
      const classes = current.className.trim().split(/\s+/).filter(c => c.length > 0);
      if (classes.length > 0) selector += "." + classes.map(c => CSS.escape(c)).join(".");
    }
    const parent = current.parentElement;
    if (parent) {
      const siblings = Array.from(parent.children).filter(
        s => s.tagName === current.tagName
      );
      if (siblings.length > 1) {
        const idx = siblings.indexOf(current) + 1;
        selector += `:nth-of-type(${idx})`;
      }
    }
    path.unshift(selector);
    current = current.parentElement;
  }
  return path.join(" > ");
}

function getElementBySelector(selector) {
  try { return document.querySelector(selector); } catch { return null; }
}

function showRecorderBar() {
  let bar = document.getElementById("doda-recorder-bar");
  if (bar) return;
  bar = document.createElement("div");
  bar.id = "doda-recorder-bar";
  bar.style.cssText = `
    position: fixed; top: 0; left: 0; right: 0; z-index: 2147483647;
    background: #e94560; color: #fff; padding: 6px 12px;
    font: 13px/1.4 system-ui, sans-serif;
    display: flex; align-items: center; gap: 10px;
    box-shadow: 0 2px 8px rgba(0,0,0,0.3);
  `;
  bar.innerHTML = `
    <span style="font-weight:600;">🔴 Recording</span>
    <span id="doda-step-count" style="opacity:0.8;">0 steps</span>
    <span style="flex:1;"></span>
    <span style="font-size:11px;opacity:0.7;">Click, type, navigate — actions are being recorded</span>
  `;
  document.body.prepend(bar);
  // Push page content down
  document.body.style.marginTop = "32px";
}

function hideRecorderBar() {
  const bar = document.getElementById("doda-recorder-bar");
  if (bar) bar.remove();
  document.body.style.marginTop = "";
}

function updateStepCount() {
  const el = document.getElementById("doda-step-count");
  if (el) el.textContent = `${recordedSteps.length} steps`;
}

// === PLAYBACK ===
async function playStep(step) {
  // Navigate first if URL changed
  if (step.url && step.url !== location.href) {
    // Store remaining steps and navigate
    return { needsNavigation: step.url };
  }

  const el = getElementBySelector(step.selector);
  if (!el) return { error: `Element not found: ${step.selector}` };

  switch (step.type) {
    case "click":
      el.scrollIntoView({ block: "center" });
      await sleep(200);
      el.click();
      break;
    case "mousedown":
    case "mouseup":
      break; // skip raw mouse events, only click matters
    case "change":
    case "input":
      if (step.value !== null) {
        el.focus();
        el.value = step.value;
        el.dispatchEvent(new Event("input", { bubbles: true }));
        el.dispatchEvent(new Event("change", { bubbles: true }));
      }
      break;
    case "scroll":
      window.scrollTo({ top: step.y || 0, left: step.x || 0, behavior: "smooth" });
      break;
    case "keydown":
      if (step.key) {
        el.dispatchEvent(new KeyboardEvent("keydown", { key: step.key, bubbles: true }));
      }
      break;
    case "submit":
      if (el.tagName === "FORM") el.submit();
      break;
  }
  return { ok: true };
}

function sleep(ms) { return new Promise(r => setTimeout(r, ms)); }

// === MESSAGE HANDLER ===
browser.runtime.onMessage.addListener((msg, sender, sendResponse) => {
  switch (msg.action) {
    case "record_start":
      recording = true;
      recordedSteps = [];
      document.addEventListener("click", handleEvent, true);
      document.addEventListener("change", handleEvent, true);
      document.addEventListener("submit", handleEvent, true);
      document.addEventListener("scroll", handleEvent, true);
      // capture navigation via URL changes
      showRecorderBar();
      sendResponse({ ok: true });
      break;

    case "record_stop":
      recording = false;
      document.removeEventListener("click", handleEvent, true);
      document.removeEventListener("change", handleEvent, true);
      document.removeEventListener("submit", handleEvent, true);
      document.removeEventListener("scroll", handleEvent, true);
      hideRecorderBar();
      sendResponse({ ok: true, data: { steps: recordedSteps } });
      break;

    case "playback_step":
      if (!playbackActive) playbackActive = true;
      playStep(msg.data).then(sendResponse);
      return true;

    case "playback_done":
      playbackActive = false;
      break;

    case "playback_error":
      playbackActive = false;
      console.error("Playback error:", msg.data);
      break;

    case "repl_eval":
      try {
        const result = eval.call(null, msg.data.code);
        const output = result !== undefined ? String(result) : "undefined";
        sendResponse({ ok: true, data: output });
      } catch (e) {
        sendResponse({ ok: false, error: e.toString() });
      }
      break;
  }
});
