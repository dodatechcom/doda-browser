# Doda Rebase Guide

This file catalogs every change made to the Firefox source tree.
Use it when rebasing onto a new mozilla-central release.

## Overview

| Category | Count |
|----------|-------|
| New component directories | 6 |
| New source files | ~60+ |
| Modified upstream files | 10 |
| New about: pages | 5 |
| New JSWindowActors | 4 |
| New Rust crate | 1 |
| Default pref changes | 80+ |

---

## Group A: NEW FILES (Zero conflict risk)

These files don't exist upstream. On rebase, just copy them into the new tree.

### A1 — AI Engine (`toolkit/components/aiengine/`)
| File | Notes |
|------|-------|
| `moz.build` | Build config |
| `components.conf` | XPCOM registration |
| `jar.mn` | Chrome manifest |
| `nsIAIEngineService.idl` | XPIDL interface |
| `AIEngine.h` / `AIEngine.cpp` | C++ service + HTTP client |
| `AIEngine.sys.mjs` | JS module wrapper |
| `AIEngineParent.sys.mjs` | JSWindowActor parent |
| `AIEngineChild.sys.mjs` | JSWindowActor child |
| `content/aboutAI.html` | about:ai page |
| `content/ai.js` | about:ai logic |
| `content/ai.css` | about:ai styles |

### A2 — Knowledge Graph (`toolkit/components/knowledgegraph/`)
| File | Notes |
|------|-------|
| `moz.build` | Build config |
| `components.conf` | XPCOM registration |
| `jar.mn` | Chrome manifest |
| `nsIKnowledgeGraphService.idl` | XPIDL interface |
| `KnowledgeGraph.h` / `KnowledgeGraph.cpp` | C++ service + SQLite FTS5 |
| `KnowledgeGraph.sys.mjs` | JS module wrapper |
| `KnowledgeGraphParent.sys.mjs` | JSWindowActor parent |
| `KnowledgeGraphChild.sys.mjs` | JSWindowActor child |
| `content/aboutKnowledgeGraph.html` | about:knowledgegraph page |
| `content/aboutKnowledgeGraph.js` | about:knowledgegraph logic |
| `content/aboutKnowledgeGraph.css` | about:knowledgegraph styles |

### A3 — Identity Wallet (`toolkit/components/identitywallet/`)
| File | Notes |
|------|-------|
| `moz.build` | Build config |
| `components.conf` | XPCOM registration |
| `jar.mn` | Chrome manifest |
| `nsIIdentityWalletService.idl` | XPIDL interface |
| `IdentityWallet.h` / `IdentityWallet.cpp` | C++ service + NSS Ed25519 |
| `IdentityWallet.sys.mjs` | JS module wrapper |
| `IdentityWalletParent.sys.mjs` | JSWindowActor parent |
| `IdentityWalletChild.sys.mjs` | JSWindowActor child |
| `content/aboutIdentity.html` | about:identity page |
| `content/identity.js` | about:identity logic |
| `content/identity.css` | about:identity styles |

### A4 — Provenance / Trust OS (`toolkit/components/provenance/`)
| File | Notes |
|------|-------|
| `moz.build` | Build config |
| `components.conf` | XPCOM registration |
| `jar.mn` | Chrome manifest |
| `nsIProvenanceService.idl` | XPIDL interface |
| `Provenance.h` / `Provenance.cpp` | C++ service + text analysis |
| `Provenance.sys.mjs` | JS module wrapper |
| `ProvenanceParent.sys.mjs` | JSWindowActor parent |
| `ProvenanceChild.sys.mjs` | JSWindowActor child |
| `content/aboutProvenance.html` | about:provenance page |
| `content/provenance.js` | about:provenance logic |
| `content/provenance.css` | about:provenance styles |

### A5 — Doda Hub (`toolkit/components/sundaram/`)
| File | Notes |
|------|-------|
| `moz.build` | Build config |
| `components.conf` | XPCOM registration (CID: `{a1b2c3d4-...}`) |
| `jar.mn` | Chrome manifest |
| `nsISundaramService.idl` | XPIDL interface |
| `SundaramService.h` / `SundaramService.cpp` | C++ facade |
| `SundaramService.sys.mjs` | JS module (health checks, search, analysis) |
| `content/aboutSundaram.html` | about:sundaram page |
| `content/sundaram.js` | about:sundaram logic |
| `content/sundaram.css` | about:sundaram styles |
| `content/commandPalette.html` | Command Palette page |
| `content/commandPalette.js` | Command Palette logic |
| `content/commandPalette.css` | Command Palette styles |

### A6 — Doda Rust FFI (`toolkit/components/sundaram_rust/`)
| File | Notes |
|------|-------|
| `moz.build` | Build config (`FINAL_LIBRARY = "xul"`) |
| `Cargo.toml` | Rust crate manifest (depends on `nserror`, `nsstring`) |
| `src/lib.rs` | 6 exported functions |
| `SundaramRust.h` | `extern "C"` declarations + `mozilla::sundaram` wrappers |
| `SundaramRust.cpp` | Linkage anchor (`EnsureRustLinked()`) |

### A7 — Branding Files (`browser/branding/unofficial/`)
All files in this directory are Doda-specific replacements. On rebase, replace the entire directory with our version. Key differences:
- `configure.sh` — `MOZ_APP_DISPLAYNAME=Doda`, `MOZ_MACBUNDLE_ID=io.doda.browser`
- `locales/en-US/brand.ftl` — All 6 brand strings → Doda
- `locales/en-US/brand.properties` — All 3 brand strings → Doda
- `pref/firefox-branding.js` — 80+ privacy/UI default prefs

### A8 — Preferences Profiles (`browser/app/profile/doda.js`)
If this file exists, it contains Doda-specific pref overrides.

### A9 — Build Config
| File | Notes |
|------|-------|
| `mozconfig` | Top-level build config: `--with-app-name=Doda`, `--with-branding=browser/branding/unofficial`, update channel `Doda`, debug flags, `CCACHE`, `-j8` |

---

## Group B: MODIFIED FILES (Conflict risk on rebase)

These are upstream Firefox files that we patched. Each will need manual merge.

### B1 — `toolkit/components/moz.build`
**Change:** Added 6 DIRS entries
```python
DIRS += [
    ...
    "aiengine",        # ADDED
    ...
    "identitywallet",  # ADDED
    ...
    "knowledgegraph",  # ADDED
    ...
    "provenance",      # ADDED
    ...
    "sundaram",        # ADDED
    "sundaram_rust",   # ADDED
    ...
]
```
**Rebase:** Search for `DIRS += [` in the file; add our 6 entries alphabetically among existing entries.

### B2 — `toolkit/modules/ActorManagerParent.sys.mjs`
**Change:** Added 4 JSWindowActor blocks
```javascript
// ADDED: AIEngine actor
{
  name: "AIEngine",
  sid: "a1b2c3d4-...",
  contracts: ["@mozilla.org/aiengine/actor;1"],
  ...urls: { matches: ["http://*/*", "https://*/*"] },
  ...scripts: { parent: { module: "resource://gre/modules/AIEngineParent.sys.mjs" },
                child: { module: "resource://gre/modules/AIEngineChild.sys.mjs" } },
},
// ADDED: IdentityWallet actor (same pattern)
// ADDED: KnowledgeGraph actor (same pattern, also listens DOMContentLoaded)
// ADDED: Provenance actor (same pattern)
```
**Rebase:** Find the list of actor definitions; add our 4 blocks. The UUIDs don't matter much but should be unique.

### B3 — `docshell/base/nsAboutRedirector.cpp`
**Change:** Added 5 entries to `kRedirMap`
```cpp
  {"ai", "chrome://aiengine/content/aboutAI.html",
   nsIAboutModule::ALLOW_SCRIPT | nsIAboutModule::IS_SECURE_CHROME_UI},
  {"identity", "chrome://identitywallet/content/aboutIdentity.html",
   nsIAboutModule::ALLOW_SCRIPT | nsIAboutModule::IS_SECURE_CHROME_UI},
  {"knowledgegraph", "chrome://knowledgegraph/content/aboutKnowledgeGraph.html",
   nsIAboutModule::ALLOW_SCRIPT | nsIAboutModule::IS_SECURE_CHROME_UI},
  {"provenance", "chrome://provenance/content/aboutProvenance.html",
   nsIAboutModule::ALLOW_SCRIPT | nsIAboutModule::IS_SECURE_CHROME_UI},
  {"sundaram", "chrome://sundaram/content/aboutSundaram.html",
   nsIAboutModule::ALLOW_SCRIPT | nsIAboutModule::IS_SECURE_CHROME_UI},
```
**Rebase:** Find `kRedirMap` array; add 5 entries. Entries can go anywhere in the array.

### B4 — `docshell/build/components.conf`
**Change:** Added 5 entries to `about_pages` list
```python
about_pages = [
    ...
    'ai',          # ADDED
    'identity',    # ADDED
    'knowledgegraph',  # ADDED
    'provenance',  # ADDED
    'sundaram',    # ADDED
]
```
**Rebase:** Find the `about_pages` list; add 5 entries alphabetically.

### B5 — `browser/moz.configure`
**Change:**
- Set `MOZ_APP_VENDOR` to `Doda`
- Set `MOZ_APP_ID` to `{73616d2e-6475-7261-6d2d-62726f77736572}`
- Disabled healthreport, sync, normandy, pocket, experiments, datapermissions, crashreporter, breakpad
**Rebase:** Search for existing `MOZ_APP_VENDOR` and `MOZ_APP_ID` settings; replace values. Search for `healthreport`, `sync`, etc. and add `imply_option("--disable-...", True)` lines.

### B6 — `browser/confvars.sh`
**Change:** (If any) Sets `MOZ_BRANDING_DIRECTORY=browser/branding/unofficial`
**Rebase:** Usually no change needed — just ensure our branding dir exists.

### B7 — `toolkit/library/rust/shared/Cargo.toml`
**Change:** Added 1 dependency
```toml
sundaram_rust = { path = "../../../components/sundaram_rust" }
```
**Rebase:** Find the `[dependencies]` section; add this line (alphabetically near the end of the in-tree crates).

### B8 — `toolkit/library/rust/shared/lib.rs`
**Change:** Added 1 `extern crate`
```rust
extern crate sundaram_rust;
```
**Rebase:** Find the block of `extern crate` declarations; add this line alphabetically.

### B9 — `Cargo.lock`
**Change:** Added `sundaram_rust` package entry
**Rebase:** Run `cargo generate-lockfile --manifest-path toolkit/library/rust/shared/Cargo.toml` after copying the crate and modifying Cargo.toml. The entry will be auto-generated. (This may take a long time due to git deps — use `--offline` if vendored.)

---

## Group C: EXTERNAL FILES (Outside firefox-source/)

These files are part of the Doda project but live outside the Firefox source tree. They won't be affected by rebasing.

| File | Notes |
|------|-------|
| `components/programmable/extension/` | The only remaining WebExtension (macro recorder + REPL) |
| `design/tokens.css` | Design tokens |
| `design/theme.css` | Shared theme |
| `scripts/` | Build/run/QA/release scripts |
| `README.md` | Project documentation |
| `AGENTS.md` | Agent guide |
| `CHANGELOG.md` | Release changelog |
| `PLAN.md` | Implementation plan |
| `REBASE.md` | This file |

---

## Quick Rebase Checklist

After pulling new mozilla-central:

1. Copy all **Group A** new directories into the new tree
2. Copy `mozconfig` to root
3. Copy `browser/branding/unofficial/` (replace entire dir)
4. Apply **Group B** modifications one by one:
   - [ ] `toolkit/components/moz.build` — add 6 DIRS
   - [ ] `toolkit/modules/ActorManagerParent.sys.mjs` — add 4 actors
   - [ ] `docshell/base/nsAboutRedirector.cpp` — add 5 about: pages
   - [ ] `docshell/build/components.conf` — add 5 about_pages
   - [ ] `browser/moz.configure` — vendor, app ID, feature flags
   - [ ] `browser/confvars.sh` — branding dir
   - [ ] `toolkit/library/rust/shared/Cargo.toml` — add dep
   - [ ] `toolkit/library/rust/shared/lib.rs` — add extern crate
   - [ ] `Cargo.lock` — regenerate
5. Run `./mach build` and verify 0 errors
6. Run `./scripts/qa.sh` to verify

Expected build time: ~1-3 hours (full) on first rebuild after rebase.
