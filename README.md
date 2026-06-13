# Doda Browser

**Beautiful by default. Yours by design.**

Doda is a privacy-first, user-sovereign fork of Mozilla Firefox. It reimagines the browser as a platform for self-sovereign identity, local-first intelligence, and transparent web provenance.

## Core USPs

| USP | Description |
|-----|-------------|
| **User-Owned Identity** | Built-in DID/VC wallet compiled into the browser. Log into sites with your browser identity using NSS Ed25519 keys. Grant/revoke data access per-site. No Big Tech OAuth gatekeepers. |
| **AI-Native Assistant** | Built-in C++ XPCOM service with Ollama HTTP client, about:ai page. Summarize pages, answer questions, draft replies — all private, offline. |
| **Web Provenance** | Local agent traces content sources, flags AI-generated media, shows manipulation history. A "nutrition label" for every page. |
| **Programmable Browser** | Built-in macro recorder + scripting REPL. Automate web tasks visually without extensions. |
| **Local Knowledge Graph** | History, bookmarks, notes, highlights as a queryable graph database. Full-text + semantic search across everything you've browsed. |
| **Privacy-Hardened + Usable** | Aggressive privacy without breaking the web. Smart fingerprinting protection, tracker blocking, and anti-fingerprinting — all on by default. |
| **Trust Badge** | URL bar badge shows green/yellow/red trust score for every page. Click to open Trust OS analysis. |
| **Inline AI** | Right-click any text → Explain, Debate, or Summarize. Results appear in-browser without leaving the page. |
| **Auto Notes** | Right-click → "Save to Memory" stores AI-summarized highlights directly into the Knowledge Graph. |
| **Auto Fact-check** | Suspicious pages trigger automatic AI fact-checking on load. Results shown via notification bar. |

## Project Status

| Phase | Status | Details |
|-------|--------|---------|
| **0. Foundation** | ✅ | Mercurial clone of mozilla-central (12GB), build toolchain, project structure |
| **1. Rebranding** | ✅ | All strings → Doda, custom vendor/ID, update channel, placeholder logos |
| **2. Privacy Hardening** | ✅ | 100+ prefs hardened, telemetry/Pocket/Sync/studies disabled at build level |
| **3. Knowledge Graph** | ✅ | Native C++ service with SQLite FTS5, JSWindowActor content capture, about:knowledgegraph page |
| **4. AI Assistant** | ✅ | Native C++ service with HTTP Ollama client, about:ai chat page, JSWindowActor content extraction |
| **5. User-Owned Identity** | ✅ | Native C++ service with NSS Ed25519, SQLite wallet, about:identity page |
| **6. Web Provenance** | ✅ | Native C++ service with content fingerprinting, ngram analysis, Shannon entropy, Ollama deep analysis via HTTP, about:provenance page |
| **7. Programmable Browser** | ✅ | Macro recorder, scripting REPL, playback engine |
| **8. Polish & Release** | ✅ | Unified theme, QA suite, build release script, CHANGELOG, AGENTS.md |
| **9. Intelligence Layer** | ✅ | Unified `nsIDodaService` C++ facade, `about:doda` hub (Sync tab → 6 tabs), Command Palette (`Ctrl+Shift+,`) |
| **10A. Memory Engine (KG Upgrade)** | ✅ | Annotations CRUD, entity extraction/storage/query, timeline range queries, graph data (force-directed nodes/edges), SQLite schema upgrade (`moz_annotations`, `moz_entities`, `moz_page_entities`), about:knowledgegraph with 5 tabs (Search, Annotations, Graph, Entities, Timeline) |
| **10B. Trust OS (Provenance Upgrade)** | ✅ | Claim-level extraction & scoring, cross-source comparison, temporal spread analysis, 3 new SQLite tables (`claims`, `sources`, `claim_occurrences`), about:provenance renamed "Trust OS" with 8 tabs (Claims, Sources, Spread) |
| **10C. Passwordless Identity (Wallet Upgrade)** | ✅ | Login dashboard, per-site permission grants, friendly login names, active session tracking, renamed "Passwordless Identity", no DID jargon in UI |
| **10D. Contextual AI (AI Engine Upgrade)** | ✅ | Tab-aware context gathering, Knowledge Graph integration, cross-page reasoning, debate mode (skeptic/balanced/creative/concise/detailed), `generateWithContext()` C++ method, mode selector UI, context sources panel |
| **11A. Rust Core Engine** | ✅ | Rust crate with 6 FFI functions (normalize, hash, trigrams, entropy, claim score, entities), compiled via `gkrust-shared` into `libxul.so`, C++ wrapper at `toolkit/components/sundaram_rust/` |
| **11B. Encrypted Private Sync** | ✅ | Export/import all data (KG, Provenance, Identity, AI) as encrypted `.doda` bundles. AES-256-GCM + PBKDF2 via Web Crypto API. Sync tab in about:doda. |
| **11C. Mobile Strategy** | 🔮 | Basic Android build, companion to desktop. Identity + Memory + Trust. |
| **11D. Stability & Reliability** | ✅ | DB backup before schema migration, memory pressure handler, cached Ollama health checks, periodic connection polling, real sub-service status |
| **12. Browser Intelligence Features** | ✅ | Trust badge in URL bar (green/yellow/red), inline AI (right-click → Explain/Debate/Summarize), auto notes (Save to Memory via KG), auto fact-check on suspicious pages |

## Quick Start

### Prerequisites
- **OS**: Linux (Windows/macOS also supported by mozilla-central)
- **Disk**: 40GB+ free
- **RAM**: 16GB+ recommended
- **Tools**: rustc 1.95+, clang 18+, python 3.12+, mercurial 7.2+

### Build from Source

```bash
# 1. Bootstrap (clones mozilla-central, applies Doda overlay, builds)
./scripts/bootstrap.sh

# 2. Or manually:
# Clone mozilla-central
hg clone --stream https://hg.mozilla.org/mozilla-central firefox-source
# Apply Doda overlay on top
cp -a source/* firefox-source/
cp mozconfig firefox-source/mozconfig
# Build
cd firefox-source
./mach build          # First build: 1-4 hours
./mach build          # Subsequent: seconds to minutes
```

### Run

```bash
# From the firefox-source directory
cd firefox-source && ./mach run
# Navigate to any native about: page
#   about:ai — AI Assistant
#   about:knowledgegraph — Knowledge Graph / Memory Engine
#   about:identity — Passwordless Identity
#   about:provenance — Trust OS
#   about:doda — Unified Intelligence Hub

# With all extensions (Programmable only)
./scripts/run-all.sh
```

**Native components (always available, no flags needed):**
- `about:ai` — AI Assistant chat (using C++ HTTP service + Ollama)
- `about:knowledgegraph` — Knowledge Graph search
- `about:identity` — Identity Wallet management
- `about:provenance` — Web Provenance analyzer
- `about:doda` — Unified Intelligence Hub (Overview, Memory, Trust, Identity, AI, Sync)

**Keyboard shortcuts:**
- `Ctrl+Shift+,` — Open Doda Command Palette (Alfred/Spotlight-style overlay for KG search, AI, Trust checks, mode switching)

## What's Been Built

### Phase 1: Rebranding

All traces of Firefox/Nightly/Mozilla branding replaced with Doda:

| File | Change |
|------|--------|
| `browser/moz.configure` | Vendor → Doda, App ID → custom UUID, Sync/HealthReport/Studies disabled |
| `browser/confvars.sh` | Branding directory locked |
| `browser/branding/unofficial/configure.sh` | `MOZ_APP_DISPLAYNAME=Doda` |
| `browser/branding/unofficial/locales/en-US/brand.ftl` | All 6 brand strings → Doda |
| `browser/branding/unofficial/locales/en-US/brand.properties` | All 3 brand strings → Doda |
| `mozconfig` | Update channel → Doda, debug build config |

### Phase 2: Privacy Hardening

100+ preferences hardened in `browser/branding/unofficial/pref/firefox-branding.js`:

- **Telemetry**: All toolkit/telemetry, datareporting, ping-centre disabled
- **Privacy**: resistFingerprinting, tracking protection, network partitioning, first-party isolation
- **Cookies**: Block 3rd-party cookies, delete on close, no lifetime
- **WebRTC/Media**: PeerConnection, navigator, geo disabled
- **Pocket**: Full disable at pref level
- **Studies**: Normandy, shield, experiments disabled
- **DNS**: Prefetch off, DOH mode 3 (Cloudflare), referer spoofing
- **HTTPS**: Force HTTPS-only mode
- **UI**: Compact density, DuckDuckGo default search

### Phase 3: Knowledge Graph

Location: `toolkit/components/knowledgegraph/`

**Native C++ component** compiled into `libxul.so` — not a WebExtension. No other browser has this.

Architecture:

| Layer | File(s) | Role |
|-------|---------|------|
| **XPIDL Interface** | `nsIKnowledgeGraphService.idl` | Scriptable API: `indexPage()`, `search()`, `getRelated()`, `clear()` |
| **C++ Service** | `KnowledgeGraph.h` / `.cpp` | Opens `knowledgegraph.sqlite` in profile dir, manages FTS5 full-text search via mozStorage |
| **Public JS API** | `KnowledgeGraph.sys.mjs` | Importable module wrapping the XPCOM service |
| **JSWindowActor (child)** | `KnowledgeGraphChild.sys.mjs` | Fires on `DOMContentLoaded` for every http(s) page, extracts text via idle callback |
| **JSWindowActor (parent)** | `KnowledgeGraphParent.sys.mjs` | Receives page text, calls `indexPage()` on native service |
| **About Page** | `content/aboutKnowledgeGraph.html` | `about:knowledgegraph` — native UI with search box, results, related pages |
| **Schema** | `knowledgegraph.sqlite` | Auto-created. FTS5 virtual table with porter tokenizer |

Key features:
- **Automatic content capture** — every page you visit gets indexed via JSWindowActor (no content script, no WebExtension)
- **SQLite FTS5 full-text search** — fast, ranked search with snippet generation
- **Related pages** — domain-based clustering for "other pages like this"
- **`about:knowledgegraph`** — built-in page, no extension loading needed
- **Zero dependencies** — uses Firefox's built-in mozStorage and SQLite
- **Privacy-preserving** — all data stays in `knowledgegraph.sqlite` in your profile, never leaves your machine

The legacy WebExtension (`components/knowledge-graph/extension/`) was removed in favor of the native component.

### Phase 4: AI Assistant

Location: `toolkit/components/aiengine/`

**Native C++ component** compiled into `libxul.so` — communicates with Ollama via HTTP for private, offline AI.

Architecture:

| Layer | File(s) | Role |
|-------|---------|------|
| **XPIDL Interface** | `nsIAIEngineService.idl` | Scriptable API: `generate(prompt, systemPrompt)`, `endpoint`/`model` prefs |
| **C++ Service** | `AIEngine.h` / `.cpp` | HTTP POST to Ollama `/api/generate` via `nsIHttpChannel`, JSON body building, async Promise return |
| **Public JS Module** | `AIEngine.sys.mjs` | Importable via `ChromeUtils.importESModule`, wraps XPCOM service with async API |
| **JSWindowActor (child)** | `AIEngineChild.sys.mjs` | Extracts page text for summarization/Q&A via `sendQuery` |
| **JSWindowActor (parent)** | `AIEngineParent.sys.mjs` | Forwards content requests between about:ai and content pages |
| **About Page** | `content/aboutAI.html` | `about:ai` — renamed **"Contextual AI"** with 4 modes: Chat, Tab Context, Cross-Page, Debate |

Key features:
- **HTTP to Ollama** — POSTs to `/api/generate` with `stream:false` for single-response completions
- **Config stored in prefs** — `doda.aiengine.endpoint` and `doda.aiengine.model` persisted via `nsIPrefBranch`
- **`about:ai`** — renamed **"Contextual AI"** (Phase 10D): 4-mode UI, context sources panel, debate rendering
- **4 modes** (Phase 10D):
  - **Chat** — standard Q&A (unchanged)
  - **Tab Context** — injects current tab content as context via `generateWithTabContext()`
  - **Cross-Page** — gathers content from all open tabs + queries Knowledge Graph, synthesizes across sources via `generateCrossPage()`
  - **Debate** — sends same prompt to Ollama with 5 different system prompts (Skeptic, Balanced, Creative, Concise, Detailed), renders each perspective side-by-side
- **Context sources panel** (Phase 10D) — expandable panel shows which tabs and KG entries were used as context
- **`generateWithContext()` C++ method** (Phase 10D) — takes pre-built context JSON, inserts it before the user's prompt for richer answers
- **Prompt+system prompt** — supports both user prompts and system instructions
- **JS module API** — `AIEngine.generate(prompt, systemPrompt)` returns Promise with parsed JSON response
- **Page content extraction** — JSWindowActor child extracts page text on demand for summarization/QA

Requires [Ollama](https://ollama.com) running locally (`ollama serve`).

The legacy WebExtension (`components/ai-engine/extension/`) is deprecated in favor of the native component.

### Phase 5: User-Owned Identity → Passwordless Identity

Location: `toolkit/components/identitywallet/`

**Native C++ component** compiled into `libxul.so` — uses NSS (Network Security Services) for Ed25519 crypto, SQLite for persistence.

Architecture:

| Layer | File(s) | Role |
|-------|---------|------|
| **XPIDL Interface** | `nsIIdentityWalletService.idl` | Scriptable API: `createIdentity`, `listIdentities`, `sign`, `verify`, `createCredential`, `grantSitePermission`, `revokeSitePermission`, `listSitePermissions`, `getActiveLogin`, `setActiveLogin`, `clearLogin`, `getLoginName`, `setLoginName` |
| **C++ Service** | `IdentityWallet.h` / `.cpp` | Ed25519 key generation via `PK11_GenerateKeyPair(CKM_EC_EDWARDS_KEY_PAIR_GEN)`, DER-encoded PKCS#8 key storage in `identity-wallet.sqlite`, signing via `PK11_SignWithMechanism(CKM_EDDSA)`, permission grants, login sessions |
| **Public JS Module** | `IdentityWallet.sys.mjs` | Importable via `ChromeUtils.importESModule`, wraps XPCOM service with async/promise API |
| **JSWindowActor (child)** | `IdentityWalletChild.sys.mjs` | Injects "Passwordless Login" button next to password fields, checks active login on each site |
| **JSWindowActor (parent)** | `IdentityWalletParent.sys.mjs` | Handles sign requests from content process |
| **About Page** | `content/aboutIdentity.html` | `about:identity` — renamed **"Passwordless Identity"**, 4 tabs: Login Dashboard, My Identities, Credentials, Settings |

Key features:
- **Friendly login names** — each identity has a human-readable name (e.g. "Personal", "Work Account") instead of raw `did:key:z...` identifiers; no DID jargon in the UI (Phase 10C)
- **Login Dashboard** (Phase 10C) — shows all sites with granted access, current site status, one-click revoke
- **Per-site permissions** (Phase 10C) — `site_permissions` table tracks which identity each site uses; auto-granted on first sign-in
- **Active login sessions** (Phase 10C) — `active_logins` table tracks currently logged-in sites for quick status display
- **NSS Ed25519 keys** — generated and managed by Firefox's own crypto library, same NSS that powers TLS
- **Secure key storage** — private keys stored as PKCS#8 DER BLOBs in `identity-wallet.sqlite`
- **`did:key:` derivation** — SHA-256 of public key → base64url → `did:key:z<id>`
- **Passwordless Login** — content script detects password fields, injects green "Passwordless Login" button, signs domain-specific challenges, auto-sets active login and permission
- **`about:identity`** — built-in page, no extension loading needed
- **Zero dependencies** — uses only NSS and mozStorage already in Firefox

The legacy WebExtension (`components/identity-wallet/extension/`) was removed in favor of the native component.

### Phase 11A: Rust Core Engine

Location: `toolkit/components/sundaram_rust/`

**First Rust crate compiled into libxul** — 6 C-ABI FFI functions for hot-path text analysis, replacing equivalent C++ implementations.

Architecture:

| Layer | File(s) | Role |
|-------|---------|------|
| **Rust Crate** | `Cargo.toml` / `src/lib.rs` | 6 `#[no_mangle] pub extern "C"` functions |
| **FFI Header** | `DodaRust.h` | `extern "C"` declarations + inline C++ wrappers in `mozilla::sundaram` namespace |
| **C++ Anchor** | `DodaRust.cpp` | `EnsureRustLinked()` forces linkage into libxul |
| **Build Integration** | `gkrust-shared/Cargo.toml` + `lib.rs` | Path dependency + `extern crate` |

FFI Functions:

| Function | Returns | Purpose |
|----------|---------|---------|
| `sundaram_normalize_text()` | `nsresult` | Lowercase + filter alphanumeric/space/dash/apostrophe |
| `sundaram_djb2_hash()` | `uint64_t` | DJB2 hash of input string |
| `sundaram_count_trigram_reps()` | `uint32_t` | Count character-level trigram repetitions |
| `sundaram_compute_entropy()` | `double` | Shannon entropy of byte distribution |
| `sundaram_claim_score()` | `double` | Combined confidence score (0-1) using entropy + length + trigram penalty |
| `sundaram_extract_entities()` | `nsresult` | Comma-separated entity extraction (3+ char alpha words) |

### Phase 11D: Stability & Reliability

**Profile recovery:** Both Knowledge Graph and Provenance now back up their SQLite databases (`.sqlite` → `.sqlite.bak`) before creating or migrating schema, preventing data loss on version changes.

**Memory pressure:** Knowledge Graph registers for the `memory-pressure` observer topic and calls `PRAGMA shrink_memory` to release unused SQLite memory when the system is under memory pressure.

**Graceful Ollama offline handling:** A new periodic health check (60s interval via SundaramService.sys.mjs) pings Ollama's `/api/tags` endpoint and caches the connection state in `doda.aiengine.connected` pref. `AIEngine::CheckHealth()` reads this cached value. The UI shows the connection status without waiting for a timeout.

**Real component status:** `SundaramService::GetStatus()` now queries each sub-service individually — AI uses `CheckHealth()`, others verify `do_GetService` succeeds — instead of returning hardcoded `true` values.

### Phase 12: Browser Intelligence Features

**Front-end features** (built entirely in JS/CSS — no C++ changes needed):

#### Trust Badge (URL bar)

A small shield icon in the URL bar shows the trust level of every page:

- **Green** (score >= 70%) — trustworthy content
- **Yellow** (score 40-70%) — some trust concerns
- **Red** (score < 40%) — likely unreliable or AI-generated

Hover shows a tooltip with the assessment. Click opens `about:provenance` for full Trust OS analysis. The badge automatically analyzes every page load via the Provenance C++ service.

Files: `browser-trustBadge.js`, `identity-block.css`, `doda-trust-badge.svg`

#### Inline AI (Right-click → Explain / Debate / Summarize)

When you select text on any page, right-click to find:

- **Explain Selection** — AI explains the selected text clearly
- **Debate Selection** — AI presents multiple perspectives (skeptic, balanced, creative, concise, detailed)
- **Summarize Selection** — AI provides a concise summary
- **Save to Memory** — AI summarizes then stores in the Knowledge Graph as an annotation

Results appear in the notification bar — no need to open a separate tab or page.

#### Auto Fact-check

When a page loads with a high AI-generation probability (>50%), Doda automatically triggers an AI fact-check via Ollama (if running). Results are shown in the notification bar alongside the trust badge update.

#### Auto Notes ("Save to Memory")

Right-click selected text → "Save to Memory" to:

1. AI-summarize the selection
2. Store the summary as a Knowledge Graph annotation
3. Instantly available via `about:knowledgegraph` search

### Phase 7: Programmable Browser

Location: `components/programmable/extension/`

A WebExtension that turns the browser into an automation platform:

- **Macro Recorder**: Records clicks, form entries, scrolls, and navigation on any page. Uses CSS selector-based targeting for robust playback.
- **Playback Engine**: Replays recorded macros step-by-step with configurable delays. Abort-safe.
- **Recording UI**: Toolbar popup with Record/Stop/Play buttons, live step counter, and visual step log.
- **Macro Storage**: Save/load/delete named macros in browser.storage.local with timestamps.
- **Scripting REPL sidebar** (`sidebar_action`): Evaluate arbitrary JavaScript in the active page context with Shift+Enter. Output panel with error highlighting, command history (↑/↓ arrows), keyboard shortcuts (Esc to clear), and quick-insert buttons for `document`, `window`, `$()`, `fetch()`.
- **Recording overlay**: Red status bar at top of page with visible recording indicator and step counter.

### Phase 8: Polish & Release

- **Design tokens**: `design/tokens.css` — canonical Doda design system (colors, typography, spacing, radii)
- **Shared theme**: `design/theme.css` — unified theme distributed to both remaining extensions, providing consistent UI components (buttons, tabs, inputs, cards, status badges, scrollbars, spinners)
- **QA suite**: `scripts/qa.sh` — 27 automated checks (manifest validity, required files, JS syntax, branding, scripts)
- **Release pipeline**: `scripts/build-release.sh` — one-command build + .xpi packaging + git tag
- **Documentation**: CHANGELOG.md, AGENTS.md (agent guide with build/run/QA commands)

### Phase 6: Web Provenance → Trust OS

Location: `toolkit/components/provenance/`

**Native C++ component** compiled into `libxul.so` — not a WebExtension. Provides a "nutrition label" for every page you visit:

- **AI content detection**: Analyzes text for AI-generation signals (ngram repetition, entropy, sentence variance, readability) and computes a probability score — all computed natively in C++
- **Ollama-powered deep analysis**: Optionally sends page content to a local LLM for deeper classification (human vs AI, with reasoning) via C++ HTTP client
- **Image provenance**: Detects likely AI-generated or stock images by analyzing filenames, dimensions, and metadata hints
- **Content fingerprinting**: DJB2-hash based fingerprint of page content for cross-site source matching
- **Trust score**: Composite metric aggregating all signals into a single 0-100% value
- **Claim-level extraction (Phase 10B)**: Splits page text into factual claims, scores each for confidence using trigram repetition + entropy + word count heuristics
- **Source comparison (Phase 10B)**: Matches claims across different URLs/domains, computes per-domain reliability scores based on historical claim accuracy
- **Spread analysis (Phase 10B)**: Tracks how claims propagate across the web over time with temporal visualization
- **SQLite schema (Phase 10B)**: 3 new tables — `claims` (normalized + hashed claims), `sources` (domain reliability), `claim_occurrences` (cross-reference)
- **Native about:provenance page** (renamed **"Trust OS"** in Phase 10B): 8-tab UI — Overview, Claims, Sources, Spread, Signals, AI Analysis, Settings, History
- **Auto-analysis**: Provenance scored automatically on every page load via JSWindowActor (no content script needed)
- **SQLite caching**: Last 200 results stored in `provenance.sqlite` with automatic eviction

### Components

| Component | Location | Language | Status |
|-----------|----------|----------|--------|
| **Identity Wallet** | **`toolkit/components/identitywallet/`** | **C++ + JS** | **✅ Native — compiled into libxul.so, NSS Ed25519, SQLite, about:identity** |
| **Knowledge Graph** | **`toolkit/components/knowledgegraph/`** | **C++ + JS** | **✅ Native — compiled into libxul.so, FTS5 search, about:knowledgegraph** |
| **AI Assistant** | **`toolkit/components/aiengine/`** | **C++ + Ollama** | **✅ Native — compiled into libxul.so, HTTP to Ollama, about:ai** |
| **Web Provenance** | **`toolkit/components/provenance/`** | **C++ + Ollama** | **✅ Native — compiled into libxul.so, text analysis + HTTP, about:provenance** |
| **Rust Core** | **`toolkit/components/sundaram_rust/`** | **Rust + C++** | **✅ Native — 6 FFI functions compiled via gkrust-shared into libxul.so** |
| **Unified Facade** | **`toolkit/components/sundaram/`** | **C++ + JS** | **✅ Native — nsIDodaService, about:doda hub, command palette** |
| Programmable Browser | `components/programmable/` | JS | ✅ WebExtension working (macro recorder, scripting REPL) |
| **Design System** | `design/` | CSS | ✅ Shared theme.css across both extensions |

### Configuration

All Doda customization is centralized in these files:

| File | Purpose |
|------|---------|
| `mozconfig` | Build options (debug, update channel, parallel build) |
| `browser/moz.configure` | Vendor, app ID, feature flags |
| `browser/branding/unofficial/pref/firefox-branding.js` | All privacy/UI default prefs |
| `browser/branding/unofficial/configure.sh` | Display name |
| `patches/rebrand.patch` | Rebranding patch reference |
| `patches/privacy-hardening.patch` | Privacy patch reference |

## Development Workflow

```bash
# Edit source files, then:
./mach build                    # Incremental build
./mach run                      # Launch
./mach build && ./mach run      # Build + launch

# Faster iteration for front-end changes only:
./mach build faster
```

## Key Directories

```
Doda/
├── firefox-source/               # Mozilla Central clone (12GB)
│   ├── browser/
│   │   ├── brandings/unofficial/ # Our branding (logos, prefs, locales)
│   │   ├── moz.configure         # App config (vendor, ID, features)
│   │   └── app/profile/          # Core preferences
│   ├── toolkit/                  # Platform components
│   │   ├── components/knowledgegraph/  # Native Knowledge Graph (C++ + FTS5 + about:page)
│   │   ├── components/identitywallet/  # Native Identity Wallet (C++ + NSS + about:page)
│   │   ├── components/aiengine/        # Native AI Engine (C++ + Ollama + about:page)
│   │   ├── components/sundaram_rust/   # Rust FFI crate (6 functions, gkrust-shared)
│   │   └── components/sundaram/        # Unified Intelligence Layer facade
│   ├── mozconfig                 # Our build config
│   └── mach                      # Build tool
├── components/
│   └── programmable/               # Macro Recorder + REPL WebExtension
├── design/                       # Design tokens, shared theme.css
├── patches/                      # Mercurial patch queue
├── scripts/
│   ├── bootstrap.sh              # Full setup pipeline
│   ├── setup-dev.sh              # Dev env checker
│   ├── build-release.sh          # Build + package + tag
│   ├── qa.sh                     # 22-check QA suite
│   ├── run-with-kg.sh            # Knowledge Graph is native — runs plain browser
│   ├── run-with-ai.sh            # AI Assistant is native — runs plain browser
│   ├── run-with-provenance.sh    # Launch with Provenance
│   ├── run-with-programmable.sh  # Launch with Programmable Browser
│   └── run-all.sh                # Launch with Programmable extension
├── designs/branding-spec.md      # Color palette, typography
├── README.md                     # This file
└── PLAN.md                       # 26-week implementation plan
```

## Resources

- **Mozilla Central source**: `hg clone https://hg.mozilla.org/mozilla-central`
- **Firefox build docs**: https://firefox-source-docs.mozilla.org/setup/
- **License**: MPL-2.0

## Marketing / Landing Page

### Doda Browser

A browser that thinks, remembers, and verifies.

Beautiful by default. Yours by design.

Doda is a next-generation browser built for a world of AI, misinformation, and data control. It doesn't just load web pages — it helps you understand, trust, and own your digital life.

### What Makes Doda Different?

Most browsers are passive. Doda is active. It:

- **Remembers** what you read
- **Helps you think** with AI (privately)
- **Verifies** what you see
- **Lets you log in** without passwords

### The 3 Pillars of Doda

#### Memory — Your Second Brain

Your browser becomes a knowledge system.

- Automatically captures pages you visit
- Full-text + semantic search across everything
- Timeline view of your thinking
- Entity extraction and connections between ideas

"Search your browsing history like Google — but private."

#### Intelligence — AI That Works With You

Built-in AI, running locally.

- Summarize any page instantly
- Ask questions about what you're reading
- Compare across multiple tabs
- Debate mode: see multiple perspectives

No cloud. No tracking. Just intelligence.

#### Trust — Know What to Believe

A "nutrition label" for the internet.

- Detect AI-generated content
- Break pages into factual claims
- Compare sources across the web
- Track how information spreads

Don't just read the web. Verify it.

### Passwordless Identity

Log in without passwords. Without Big Tech.

- Built-in identity wallet
- One-click login to supported sites
- Per-site permissions
- Full control over your data

You own your identity — not platforms.

### Automate the Web

Turn your browser into a tool, not just a viewer.

- Record actions as macros
- Replay workflows automatically
- Run scripts directly in pages
- Command palette for everything

If you can do it once, Doda can do it forever.

### Privacy by Default

No compromises. No hidden tracking.

- Zero telemetry
- Strong anti-fingerprinting
- Tracker and cookie isolation
- All data stored locally

Your data never leaves your machine.

### Built for Power Users

Doda is designed for people who:

- research deeply
- think critically
- value privacy
- want control over their tools

Whether you're a developer, researcher, or creator — Doda becomes your thinking workspace.

### How It Works

Under the hood:

- Built on a hardened Firefox core
- Native C++ components (not extensions)
- Rust-powered performance engine
- Local AI via Ollama
- SQLite-backed knowledge and trust systems

Fast. Private. Fully integrated.

### Get Started

```bash
./mach build
./mach run
```

Then explore:

- `about:ai` — AI Assistant
- `about:knowledgegraph` — Memory Engine
- `about:provenance` — Trust OS
- `about:identity` — Passwordless Identity

### Why Doda Exists

The web is changing.

- AI-generated content is everywhere
- Trust is collapsing
- Identity is controlled by platforms
- Browsers haven't evolved in a decade

Doda is built for what comes next.

### The Vision

Doda is not just a browser. It's:

- your memory
- your assistant
- your identity
- your filter for truth

A personal system for navigating the modern web.

### What's Next

- Mobile companion (Android)
- Smarter AI agents
- Cross-device encrypted sync
- Deeper trust analysis

The browser was never meant to stay dumb.
