# Doda Browser — Implementation Plan

## Status (May 2026)

| Phase | Title | Status |
|-------|-------|--------|
| 0 | Foundation | ✅ Complete |
| 1 | Rebranding | ✅ Complete |
| 2 | Privacy Hardening | ✅ Complete |
| 3 | Knowledge Graph → Native C++ | ✅ Complete |
| 4 | AI Assistant → Native C++ | ✅ Complete |
| 5 | Identity Wallet → Native C++ | ✅ Complete |
| 6 | Web Provenance → Native C++ | ✅ Complete |
| 7 | Programmable Browser | ✅ WebExtension (stays as-is) |
| 8 | Polish & Release | ✅ Complete |
| **9** | **Unification: Intelligence Layer** | **✅ Complete** |
| **10** | **Deep Upgrades** | **✅ Complete** |
| **11A** | **Rust Core Engine** | **✅ Complete** |
| **11B** | **Encrypted Private Sync** | **✅ Complete** |
| **11D** | **Stability & Reliability** | **✅ Complete** |
| 11C | Mobile Strategy | 🔮 Planned |

## Architectural Decision: Programmable Browser stays as WebExtension

The Programmable Browser (macro recorder + REPL) fundamentally needs JavaScript injection into web pages. WebExtensions are the correct architecture for this — a native XPCOM component would add complexity with no benefit. It will remain as `--load-extension` and may later gain a Rust-based execution sandbox.

---

## Phase 9: Doda Intelligence Layer (Unification)

**Goal:** Merge AI Assistant, Knowledge Graph, Provenance, and Identity into one cohesive system instead of 4 separate about: pages.

### 9A — Shared C++ Service (`nsIDodaService`)

Create a facade service at `toolkit/components/sundaram/` that:
- Aggregates all 4 existing services
- Provides a unified JS API: `Services.doda.*`
- Handles inter-component queries (e.g. "search KG, then summarize with AI")
- Single `EnsureReady()` that initializes all sub-services lazily

**Files:**
- `nsISundaramService.idl` — unified API
- `SundaramService.cpp/h` — facade implementation
- `SundaramService.sys.mjs` — JS wrapper
- `components.conf`, `moz.build`, `jar.mn`

### 9B — Command Palette (`Ctrl+K` / `Cmd+K`)

The universal entry point. One keystroke summons a Spotlight/Alfred-style overlay that can:
- Search KG ("find my notes on Rust")
- Ask AI ("summarize this tab")
- Check trust ("provenance of current page")
- Manage identity ("login with DID")
- Run macros
- Switch modes

**Files:**
- `toolkit/components/sundaram/content/commandPalette.html/js/css`
- Registered as `chrome://doda/content/commandPalette.html`
- Launched by a system keybinding registered in `browser/components/`

### 9C — Unified Side Panel

Replace individual about: pages with:
- A single side panel that switches between KG/AI/Provenance/Identity tabs
- Persistent across page navigations
- Can be toggled with `Ctrl+Shift+S`

**Files:**
- `toolkit/components/sundaram/content/sidePanel.html/js/css`
- Actor files for tab-level state tracking

### 9D — Modes System

Different UI presets for different workflows:
- **Research Mode**: KG sidebar + AI summarization + Provenance trust overlay
- **Focus Mode**: Minimal UI, no AI, reading-only
- **Secure Mode**: Identity visible, strict permissions, provenance on every page
- **Developer Mode**: REPL visible, macro recorder controls

Stored as a pref: `doda.mode`

### Build Cost

One incremental build for all of Phase 9: ~30-60 min

---

## Phase 10: Deep Upgrades

Each pillar gets a major capability upgrade, informed by strategic feedback.

### 10A — Memory Engine (KG v2)

Upgrade from "page indexer" to "personal memory":
- ✍️ Highlights and annotations stored per-page
- 🧠 Auto entity extraction (people, topics, organizations) via local NLP
- 🔗 Relationship mapping and graph visualization (D3.js or Canvas)
- ⏱️ Time-travel browsing — see your KG state at any past date
- 🔍 "Ask Doda" — natural language queries over your memory

### 10B — Trust OS (Provenance v2)

Upgrade from "nutrition label" to "information integrity layer":
- 🔄 Source comparison: "this claim appears on 5 other sites; original: X"
- 🧵 Spread analysis: trace how information moved across the web
- 🏷️ Per-claim trust scores (not just per-page)
- 📊 Visual timeline of information propagation
- 🔌 Pluggable fact-check APIs (optional: connect to external fact-checkers)

### 10C — Passwordless Identity (Identity v2)

Upgrade from "DID wallet" to "login without passwords anywhere":
- Simplified UX: replace "DID:key" jargon with "Your browser identity"
- 🧾 Login history dashboard — see every site you've authenticated with
- 🔒 Granular permission dashboard: "This site can access: email only"
- 🔑 Auto-provision identity on sites that support it
- Integration with passkeys / WebAuthn as fallback

### 10D — Contextual AI (AI v2)

Upgrade from "chat with page" to "AI that knows your browser":
- 🧠 Tab-aware: knows what's in all open tabs
- 📚 KG-aware: searches your memory before answering
- 🔄 Cross-page reasoning: "compare these 3 articles"
- 🗣️ Debate Mode: shows arguments FOR and AGAINST from trusted sources
- 💾 Session memory: remembers context across conversations

---

## Phase 11: Architecture & Infrastructure

### 11A — Rust Core Engine

Identify hot paths and rewrite in Rust as Gecko components:
- Macro playback engine (replaces JS-based macro runner)
- Provenance text analysis pipeline
- Knowledge Graph entity extraction
- Identity cryptographic operations (some already in NSS)

### 11B — Encrypted Private Sync

- End-to-end encrypted sync of KG, Identity, settings
- Self-hosted option (your own server)
- No-account option (local LAN sync, or peer-to-peer)

### 11C — Mobile Strategy

- Basic Android build (Firefox for Android is Gecko-based)
- Focus on: identity (passwordless login), memory (KG sync), trusted browsing (provenance)
- Not a full-featured mobile browser — companion to desktop

### 11D — Stability & Reliability

- Automated crash reporting (no telemetry — local crash logs only)
- Profile recovery tool
- Graceful degradation when Ollama is not running
- Memory pressure handling for large KGs

---

## Phase 11 Implementation Summary

### 11A — Rust Core Engine ✅ Complete

**Files created:**
- `toolkit/components/sundaram_rust/Cargo.toml` — Crate manifest (edition 2018, depends on `nserror` + `nsstring`)
- `toolkit/components/sundaram_rust/src/lib.rs` — 6 exported functions:
  - `sundaram_normalize_text()` — lowercase + filter alphanumeric
  - `sundaram_djb2_hash()` — DJB2 hash (u64)
  - `sundaram_count_trigram_reps()` — trigram repetition counter
  - `sundaram_compute_entropy()` — Shannon entropy (f64)
  - `sundaram_claim_score()` — combined confidence score (0-1)
  - `sundaram_extract_entities()` — comma-separated entity extraction
- `toolkit/components/sundaram_rust/moz.build` — FINAL_LIBRARY = "xul", exports SundaramRust.h
- `toolkit/components/sundaram_rust/SundaramRust.h` — C++ wrapper header (`extern "C"` declarations + inline C++ wrappers in `mozilla::sundaram` namespace)
- `toolkit/components/sundaram_rust/SundaramRust.cpp` — linkage anchor (`EnsureRustLinked()`)

**Files modified:**
- `toolkit/library/rust/shared/Cargo.toml` — added `sundaram_rust` path dependency
- `toolkit/library/rust/shared/lib.rs` — added `extern crate sundaram_rust;`
- `toolkit/components/moz.build` — added `sundaram_rust` to DIRS

### 11D — Stability & Reliability ✅ Complete

**Changes:**
- **KnowledgeGraph**: `BackupDatabase()` copies `.sqlite` → `.sqlite.bak` before `CreateSchema()`. New `memory-pressure` observer calls `PRAGMA shrink_memory`.
- **Provenance**: `BackupDatabase()` same pattern — backup before schema migration.
- **AIEngine**: `CheckHealth()` reads cached connection state from `doda.aiengine.connected` pref (set by JS periodic poller).
- **DodaService**: `GetStatus()` queries each sub-service individually (AI uses `CheckHealth()`, others check `do_GetService` success).
- **SundaramService.sys.mjs**: `startHealthCheck()` pings Ollama `/api/tags` every 60s and caches result via pref.
- **SundaramService.cpp**: `LazyInit()` calls `EnsureRustLinked()` to force Rust FFI linkage.

---

## Design Principles

1. **Local-first**: Everything runs on-device by default. Cloud is optional and opt-in.
2. **Progressive disclosure**: Start simple, reveal depth on demand. Command Palette → deep settings.
3. **Composable pillars**: AI can query KG. Provenance feeds AI. Identity gates Identity. They're not silos.
4. **One narrative, not six**: "Browser that understands, verifies, and remembers — privately."

---

## Key Metrics

| Metric | Current | Target (Phase 10) |
|--------|---------|-------------------|
| Distinct about: pages | 4 | 1 (Intelligence Hub) + Command Palette |
| User-facing concepts | AI, KG, Provenance, Identity, Macros, REPL | "Doda Intelligence" |
| Entry points | 4 separate URLs | 1 keyboard shortcut (`Ctrl+K`) |
| Extension count | 1 (Programmable) | 1 (Programmable) |
| Native components | 4 | 5 (+DodaService) |
