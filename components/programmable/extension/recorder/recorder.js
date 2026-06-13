// Doda Programmable Browser — recorder popup

const $ = id => document.getElementById(id);
let currentSteps = [];
let isRecording = false;

// === TABS ===
document.querySelectorAll(".tab").forEach(tab => {
  tab.addEventListener("click", () => {
    document.querySelectorAll(".tab").forEach(t => t.classList.remove("active"));
    document.querySelectorAll(".panel").forEach(p => p.classList.remove("active"));
    tab.classList.add("active");
    document.getElementById("panel-" + tab.dataset.tab).classList.add("active");
    if (tab.dataset.tab === "macros") loadMacroList();
  });
});

// === RECORDING ===
$("btnRecord").addEventListener("click", async () => {
  const resp = await browser.runtime.sendMessage({ action: "record_start" });
  if (resp.ok) {
    isRecording = true;
    currentSteps = [];
    updateUI();
  }
});

$("btnStop").addEventListener("click", async () => {
  const resp = await browser.runtime.sendMessage({ action: "record_stop" });
  if (resp.ok && resp.data) {
    currentSteps = resp.data.steps || [];
    isRecording = false;
    updateUI();
    renderSteps(currentSteps);
    if (currentSteps.length > 0) {
      $("saveSection").style.display = "flex";
    }
  }
});

$("btnPlayback").addEventListener("click", async () => {
  if (currentSteps.length === 0) return;
  $("btnPlayback").disabled = true;
  $("btnPlayback").textContent = "⏹ Stop";
  $("btnPlayback").className = "btn btn-danger";

  // Stop any existing playback
  await browser.runtime.sendMessage({ action: "playback_abort" }).catch(() => {});
  // Start playback
  const resp = await browser.runtime.sendMessage({
    action: "playback_start",
    data: { steps: currentSteps },
  });
  if (!resp.ok) {
    $("btnPlayback").disabled = false;
    $("btnPlayback").textContent = "▶ Play";
    $("btnPlayback").className = "btn btn-play";
  }

  // After a delay, re-enable
  setTimeout(() => {
    $("btnPlayback").disabled = false;
    $("btnPlayback").textContent = "▶ Play";
    $("btnPlayback").className = "btn btn-play";
  }, currentSteps.length * 500 + 2000);
});

$("btnSave").addEventListener("click", async () => {
  const name = $("macroName").value.trim();
  if (!name) return;
  const resp = await browser.runtime.sendMessage({
    action: "macro_save",
    data: { name, steps: currentSteps },
  });
  if (resp.ok) {
    $("macroName").value = "";
    $("saveSection").style.display = "none";
    loadMacroList();
  }
});

// === UI HELPERS ===
function updateUI() {
  if (isRecording) {
    $("btnRecord").textContent = "🔴 Recording...";
    $("btnRecord").classList.add("recording");
    $("btnRecord").disabled = true;
    $("btnStop").disabled = false;
    $("btnPlayback").disabled = true;
    $("statusBadge").textContent = "Recording";
    $("statusBadge").className = "status-badge recording";
  } else {
    $("btnRecord").textContent = "🔴 Record";
    $("btnRecord").classList.remove("recording");
    $("btnRecord").disabled = false;
    $("btnStop").disabled = true;
    $("btnPlayback").disabled = currentSteps.length === 0;
    $("statusBadge").textContent = "Idle";
    $("statusBadge").className = "status-badge idle";
  }
  $("stepCount").textContent = currentSteps.length;
}

function renderSteps(steps) {
  const container = $("stepList");
  if (steps.length === 0) {
    container.innerHTML = '<div class="empty-state">No steps recorded yet</div>';
    return;
  }
  container.innerHTML = steps.map((s, i) => {
    const label = { click: "🖱 Click", change: "✏️ Type", submit: "📤 Submit", scroll: "📜 Scroll" }[s.type] || s.type;
    const detail = s.value ? `"${s.value.slice(0, 40)}"` : s.text || s.href || s.selector;
    return `<div class="step-item"><span class="action">${label}</span> ${detail}</div>`;
  }).join("");
}

// === MACROS ===
async function loadMacroList() {
  const resp = await browser.runtime.sendMessage({ action: "macro_list" });
  const container = $("macroList");
  if (!resp.ok || !resp.data.length) {
    container.innerHTML = '<div class="empty-state">No saved macros</div>';
    return;
  }
  container.innerHTML = resp.data.map(m => `
    <div class="macro-item">
      <div>
        <div class="name">${m.name}</div>
        <div class="meta">${m.steps.length} steps · ${new Date(m.updated).toLocaleDateString()}</div>
      </div>
      <div class="actions">
        <button data-load="${m.name}">Load</button>
        <button data-delete="${m.name}">Del</button>
      </div>
    </div>
  `).join("");

  container.querySelectorAll("[data-load]").forEach(btn => {
    btn.addEventListener("click", async () => {
      const name = btn.dataset.load;
      const resp = await browser.runtime.sendMessage({ action: "macro_load", data: { name } });
      if (resp.ok && resp.data) {
        currentSteps = resp.data.steps || [];
        renderSteps(currentSteps);
        $("stepCount").textContent = currentSteps.length;
        $("btnPlayback").disabled = false;
        // Switch to recorder tab
        document.querySelector('[data-tab="recorder"]').click();
      }
    });
  });

  container.querySelectorAll("[data-delete]").forEach(btn => {
    btn.addEventListener("click", async () => {
      await browser.runtime.sendMessage({ action: "macro_delete", data: { name: btn.dataset.delete } });
      loadMacroList();
    });
  });
}
