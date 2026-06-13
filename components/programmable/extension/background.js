// Doda Programmable Browser — macro engine

// === SELECTOR GENERATION ===
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
  try {
    return document.querySelector(selector);
  } catch {
    return null;
  }
}

// === EVENT CAPTURE ===
function captureEvent(e) {
  const target = e.target;
  const event = {
    type: e.type,
    selector: buildSelector(target),
    tagName: target.tagName?.toLowerCase() || "",
    value: target.value !== undefined ? target.value.slice(0, 200) : null,
    text: (target.textContent || "").trim().slice(0, 100) || null,
    href: target.href || null,
    x: e.clientX || null,
    y: e.clientY || null,
    timestamp: Date.now(),
    url: location.href,
  };

  // For input/change events, capture the value after the user types
  if (e.type === "change" && target.type === "text") {
    event.value = target.value;
  }

  return event;
}

// === MACRO STORAGE ===
const STORAGE_KEY = "doda-macros";

async function saveMacro(name, steps) {
  const { [STORAGE_KEY]: raw = {} } = await browser.storage.local.get(STORAGE_KEY);
  raw[name] = { name, steps, created: Date.now(), updated: Date.now() };
  await browser.storage.local.set({ [STORAGE_KEY]: raw });
  return true;
}

async function loadMacro(name) {
  const { [STORAGE_KEY]: raw = {} } = await browser.storage.local.get(STORAGE_KEY);
  return raw[name] || null;
}

async function listMacros() {
  const { [STORAGE_KEY]: raw = {} } = await browser.storage.local.get(STORAGE_KEY);
  return Object.values(raw).sort((a, b) => b.updated - a.updated);
}

async function deleteMacro(name) {
  const { [STORAGE_KEY]: raw = {} } = await browser.storage.local.get(STORAGE_KEY);
  delete raw[name];
  await browser.storage.local.set({ [STORAGE_KEY]: raw });
}

// === PLAYBACK ===
let playbackAbort = false;

async function playbackSteps(tabId, steps) {
  playbackAbort = false;
  for (let i = 0; i < steps.length; i++) {
    if (playbackAbort) throw new Error("Playback aborted");
    const step = steps[i];
    try {
      await browser.tabs.sendMessage(tabId, {
        action: "playback_step",
        data: step,
      });
      // Wait between steps
      const delay = steps[i + 1]
        ? Math.min(steps[i + 1].timestamp - step.timestamp, 2000)
        : 500;
      await new Promise(r => setTimeout(r, Math.max(delay, 300)));
    } catch (e) {
      console.warn("Playback step failed:", step, e);
    }
  }
}

// === TABS API ===
async function getActiveTab() {
  const [tab] = await browser.tabs.query({ active: true, currentWindow: true });
  return tab;
}

// === MESSAGE HANDLER ===
browser.runtime.onMessage.addListener((msg, sender, sendResponse) => {
  const handler = {
    async record_start() {
      // Tell content script to start recording
      const tab = await getActiveTab();
      await browser.tabs.sendMessage(tab.id, { action: "record_start" });
      return { ok: true };
    },
    async record_stop() {
      const tab = await getActiveTab();
      const resp = await browser.tabs.sendMessage(tab.id, { action: "record_stop" });
      return resp;
    },
    async macro_save(data) {
      await saveMacro(data.name, data.steps);
      return { ok: true };
    },
    async macro_load(data) {
      const macro = await loadMacro(data.name);
      return { ok: true, data: macro };
    },
    async macro_list() {
      return { ok: true, data: await listMacros() };
    },
    async macro_delete(data) {
      await deleteMacro(data.name);
      return { ok: true };
    },
    async playback_start(data) {
      const tab = await getActiveTab();
      const { steps } = data;
      playbackSteps(tab.id, steps).then(() => {
        browser.tabs.sendMessage(tab.id, { action: "playback_done" }).catch(() => {});
      }).catch(e => {
        browser.tabs.sendMessage(tab.id, { action: "playback_error", data: e.message }).catch(() => {});
      });
      return { ok: true };
    },
    async playback_abort() {
      playbackAbort = true;
      return { ok: true };
    },
    async navigate(data) {
      const tab = await getActiveTab();
      await browser.tabs.update(tab.id, { url: data.url });
      return { ok: true };
    },
  };

  const fn = handler[msg.action];
  if (fn) {
    fn(msg.data).then(r => sendResponse(r)).catch(e => sendResponse({ ok: false, error: e.message }));
    return true;
  }
});
