# Doda — Agent Guide

## Phase Structure
| Phase | Title | Status |
|-------|-------|--------|
| 0 | Foundation | ✅ Complete |
| 1 | Rebranding | ✅ Complete |
| 2 | Privacy Hardening | ✅ Complete |
| 3 | Knowledge Graph → Native C++ | ✅ Complete |
| 4 | AI Assistant → Native C++ | ✅ Complete |
| 5 | Identity Wallet → Native C++ | ✅ Complete |
| 6 | Web Provenance → Native C++ | ✅ Complete |
| 7 | Programmable Browser | ✅ WebExtension (stays) |
| 8 | Polish & Release | ✅ Complete |
| **9** | **Unification: Intelligence Layer** | **✅ Complete** |
| **10A** | **Memory Engine (KG Upgrade)** | **✅ Complete** |
| **10B** | **Trust OS (Provenance Upgrade)** | **✅ Complete** |
| **10C** | **Passwordless Identity** | **✅ Complete** |
| **10D** | **Contextual AI** | **✅ Complete** |
| **11A** | **Rust Core Engine** | **✅ Complete** |
| **11B** | **Encrypted Private Sync** | **✅ Complete** |
| **11D** | **Stability & Reliability** | **✅ Complete** |
| 11C | Mobile Strategy | 🔮 Planned |

## Build
```bash
cd /home/admin1/projects/Sundaram/firefox-source
./mach build                    # Full/incremental build
./mach build 2>&1               # Suppress AI detection noise
```

## Run
```bash
cd /home/admin1/projects/Sundaram
./scripts/run-all.sh            # Programmable extension only, or plain browser
./scripts/run-with-kg.sh        # KG is native — runs plain browser, load about:knowledgegraph
./scripts/run-with-ai.sh        # AI is native — runs plain browser, load about:ai
./scripts/run-with-provenance.sh  # Provenance is native — runs plain browser, load about:provenance
./scripts/run-with-programmable.sh  # Macro Recorder + REPL (extension load)
```

## Native About Pages (always available)
- `about:ai` — Contextual AI (native C++ service, uses Ollama, 4 modes: Chat/Tab Context/Cross-Page/Debate)
- `about:knowledgegraph` — Knowledge Graph / Memory Engine (native C++ service, SQLite FTS5, annotations, entities, timeline)
- `about:identity` — Passwordless Identity (native C++ service, NSS Ed25519, login dashboard, site permissions)
- `about:provenance` — **Trust OS** (native C++ service, text analysis + Ollama, claim extraction, source comparison, spread analysis)
- `about:doda` — Unified Intelligence Hub (Phase 9 complete, Ctrl+Shift+, opens Command Palette)
- `about:tutorial` — Full onboarding guide for all features + Ollama setup
- `about:permissions` — Permission Inspector: every site permission in one place, one-click revoke

## Static About Pages (JS-only, no C++ service)
- `about:tutorial` — `toolkit/components/tutorial/` (onboarding guide)
- `about:permissions` — `toolkit/components/permissions/` (lists/revokes all site permissions via `Services.perms`)

## Built-in Extensions (compiled into browser)
- `durgashield@security.extension` — DurgaShield v1.0.0 (ad/tracker blocking, malware/phishing protection)
- Source: `browser/extensions/durgashield/` — all files copied from AMO XPI
- Registration: automatic via `gen_built_in_addons.py` scanning `builtin-addons/durgashield/`
## QA
```bash
./scripts/qa.sh                 # 40 checks: manifest, files, JS syntax, branding, theme
```

## Release
```bash
./scripts/build-release.sh v1.0.0  # Build + package .xpi + git tag
```

## Key Paths
- Firefox source: `/home/admin1/projects/Sundaram/firefox-source/`
- Build binary: `obj-x86_64-pc-linux-gnu/dist/bin/firefox`
- Mozconfig: `/home/admin1/projects/Sundaram/firefox-source/mozconfig`
- Branding: `browser/branding/unofficial/`
- Components: `/home/admin1/projects/Sundaram/components/`
- Design tokens: `/home/admin1/projects/Sundaram/design/`

## Extensions (WebExtensions, no rebuild needed)
| Component | Path | Type |
|-----------|------|------|
| Programmable | `components/programmable/extension/` | Popup + Sidebar + Content Script |

Knowledge Graph, Identity Wallet, AI Assistant, and Provenance are now **native C++ components** compiled into `libxul.so` — no extension loading needed.

## C++ Build Changes (require rebuild)
- `browser/moz.configure` — Feature flags, vendor, app ID
- `browser/branding/unofficial/pref/firefox-branding.js` — Default prefs
- `mozconfig` — Build configuration
- Branding files in `browser/branding/unofficial/`
- `toolkit/components/aiengine/` — AI Engine C++ component (HTTP + Ollama + about:ai)
- `toolkit/components/identitywallet/` — Identity Wallet C++ component (NSS + SQLite)
- `toolkit/components/knowledgegraph/` — Knowledge Graph C++ component (SQLite FTS5)
- `toolkit/components/provenance/` — Provenance C++ component (Ollama HTTP + text analysis + SQLite)
- `toolkit/components/sundaram/` — Doda Intelligence Layer facade (C++ + about:doda + command palette, Phase 9A complete)
- `toolkit/components/sundaram_rust/` — Rust FFI crate (text normalization, hashing, entity extraction, claim scoring; Phase 11A)
- `toolkit/library/rust/shared/Cargo.toml` + `lib.rs` — gkrust-shared dependency list (add new Rust crates here)

## Key Architecture Decisions
- **Programmable Browser stays as WebExtension** — macro recorder/REPL need JS page injection; XPCOM adds no benefit
- **All 4 pillars (AI, KG, Provenance, Identity) now native C++** — compiled into libxul.so
- **Phase 9 unifies them** into a single Intelligence Layer with Command Palette (`Ctrl+K`)
- **Phase 11A adds Rust FFI for hot paths** — text analysis migrated to Rust via `#[no_mangle] extern "C"` functions, linked through `gkrust-shared` into `libxul.so`
- **Phase 11D adds stability** — DB backup before schema migrations, memory pressure handling, cached Ollama health checks with periodic polling
- **No C++ work for Programmable** — focus engineering effort on unification and deep upgrades

## Native C++ Components (compiled into libxul.so)
| Component | Location | Contract ID | About Page |
|-----------|----------|-------------|------------|
| Knowledge Graph | `toolkit/components/knowledgegraph/` | `@mozilla.org/knowledgegraph/service;1` | `about:knowledgegraph` |
| Identity Wallet | `toolkit/components/identitywallet/` | `@mozilla.org/identitywallet/service;1` | `about:identity` |
| AI Engine | `toolkit/components/aiengine/` | `@mozilla.org/aiengine/service;1` | `about:ai` |
| Provenance | `toolkit/components/provenance/` | `@mozilla.org/provenance/service;1` | `about:provenance` |
| Doda Rust | `toolkit/components/sundaram_rust/` | N/A (FFI) | — |

## Notes
- Debug build output is large (libxul.so ~2.9GB). Do not strip unless packaging for release.
- First full build: 3h 17min. Incremental builds: seconds to minutes.
- AI detection may suppress terminal output. Use `2>&1` or check `.mozbuild/logs/build/` for full logs.
- Ollama endpoint: `http://127.0.0.1:11434` (default for AI Assistant + Provenance)
- The remaining extension (Programmable) is loaded via `--load-extension` flag; no installation needed.
