// Doda Scripting REPL — sidebar

const output = document.getElementById("replOutput");
const input = document.getElementById("replInput");
const evalBtn = document.getElementById("btnEval");
const clearBtn = document.getElementById("btnClear");

let history = [];
let historyIdx = -1;

function appendEntry(type, text) {
  const div = document.createElement("div");
  div.className = `entry ${type}`;
  div.textContent = text;
  output.appendChild(div);
  output.scrollTop = output.scrollHeight;
}

async function evaluate(code) {
  if (!code.trim()) return;
  appendEntry("input", `> ${code}`);
  history.push(code);
  historyIdx = history.length;

  try {
    const [tab] = await browser.tabs.query({ active: true, currentWindow: true });
    if (!tab) {
      appendEntry("error", "Error: No active tab");
      return;
    }
    const resp = await browser.tabs.sendMessage(tab.id, {
      action: "repl_eval",
      data: { code },
    });
    if (resp.ok) {
      appendEntry("output", resp.data);
    } else {
      appendEntry("error", resp.error || "Unknown error");
    }
  } catch (e) {
    appendEntry("error", String(e));
  }
}

// Eval on Shift+Enter or button click
input.addEventListener("keydown", e => {
  if (e.key === "Enter" && e.shiftKey) {
    e.preventDefault();
    evaluate(input.value);
    input.value = "";
  }
  if (e.key === "Escape") {
    input.value = "";
  }
  if (e.key === "ArrowUp") {
    e.preventDefault();
    if (historyIdx > 0) {
      historyIdx--;
      input.value = history[historyIdx];
    }
  }
  if (e.key === "ArrowDown") {
    e.preventDefault();
    if (historyIdx < history.length - 1) {
      historyIdx++;
      input.value = history[historyIdx];
    } else {
      historyIdx = history.length;
      input.value = "";
    }
  }
});

evalBtn.addEventListener("click", () => {
  evaluate(input.value);
  input.value = "";
});

clearBtn.addEventListener("click", () => {
  output.innerHTML = "";
});

// Quick shortcuts
document.getElementById("btnDoc").addEventListener("click", () => {
  input.value = "document";
  input.focus();
});

document.getElementById("btnWin").addEventListener("click", () => {
  input.value = "window";
  input.focus();
});

document.getElementById("btnSel").addEventListener("click", () => {
  input.value = "document.querySelector('')";
  // Place cursor between the quotes
  input.focus();
  input.setSelectionRange(22, 22);
});

document.getElementById("btnFetch").addEventListener("click", () => {
  input.value = "await fetch('')";
  input.focus();
  input.setSelectionRange(13, 13);
});
