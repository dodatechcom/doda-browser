# Changelog

All notable changes to the Doda Browser will be documented in this file.

## [v1.3.0] - 2026-05-18

### Added
- **Ctrl+Shift+, keyboard shortcut** to open Doda Command Palette. Registered via `browser-sets.inc.xhtml` `<key>` element + `browserSets.ftl` l10n binding + `browser-sets.js` command handler. Opens `chrome://sundaram/content/commandPalette.html` as a centered chrome window (600x500). Prevents duplicate windows by reusing existing palette instance.

### Fixed
- **Identity deletion leak**: `DeleteIdentity()` used `ExecuteSimpleSQL` with unbound `:did` for cleanup SQL — site permissions and active logins were never removed when deleting an identity. Fixed with proper `CreateStatement` + bound parameter.
- **Credential JSON injection**: `CreateCredential()` used `AppendPrintf` with raw user-provided strings in JSON context — could break credential JSON if issuer DID or subject ID contained special characters. Fixed with proper JSON escaping via lambda.
- **Dead FTS5 content sync**: `RunIndexPage()` had a `ExecuteSimpleSQL` call with unbound `:url` that was a silent no-op (the real FTS5 rebuild happens via the correct `'rebuild'` command below). Removed the dead code.

## [v1.2.0] - 2026-05-18

### Added
- **Phase 11B — Encrypted Private Sync**: Full export/import for all 4 native components (KnowledgeGraph, Provenance, Identity Wallet, AI Engine). Each component has `exportAllData()` / `importAllData()` IDL methods with JSON serialization. DodaService facade provides `exportAllBundle()` / `importAllBundle()` orchestration. New "Sync" tab in about:sundaram with AES-256-GCM + PBKDF2 encryption via Web Crypto API. Bundle files use `.sundaram` extension. C++ build: 0 errors.

## [v1.1.0] - 2026-05-18

### Added
- **Phase 11A — Rust Core Engine**: New C++/Rust FFI crate at `toolkit/components/sundaram_rust/` with 6 functions: text normalization, DJB2 hashing, trigram repetition counting, Shannon entropy, claim scoring, and entity extraction. Compiled via `gkrust-shared` into `libxul.so`. C++ wrapper via `mozilla::sundaram::*` namespace.
- **Phase 11D — Stability & Reliability**: DB backup before schema migration (KnowledgeGraph + Provenance), memory pressure handler (PRAGMA shrink_memory on `memory-pressure` observer), cached Ollama connection health check (pref `sundaram.aiengine.connected`), periodic health polling via DodaService.sys.mjs, improved `GetStatus()` with real sub-service health checks.

### Fixed
- `DodaService::GetStatus()` now returns actual component health instead of hardcoded `true`
- `AIEngine::CheckHealth()` reads cached connection state from prefs instead of just checking endpoint existence

## [v1.0.0] - 2026-05-17

### Added
- **Phase 0: Foundation** — Mercurial clone of mozilla-central, build toolchain, project structure
- **Phase 1: Rebranding** — All brand strings, app ID, vendor, and logos changed to Doda
- **Phase 2: Privacy Hardening** — 100+ privacy prefs hardened, telemetry/Pocket/Sync/studies disabled at build level
- **Phase 3: Knowledge Graph** — WebExtension sidebar with full-text search over browsing history, page indexer
- **Phase 4: AI Assistant** — Ollama-backed sidebar for page summarization, Q&A, and smart tab grouping
- **Phase 5: User-Owned Identity** — Ed25519 DID wallet, Verifiable Credential management, sign-in protocol with content script injection
- **Phase 6: Web Provenance** — Content source tracing, AI-generated content detection, nutrition label popup with trust scores
- **Phase 7: Programmable Browser** — Macro recorder (click/type/scroll/navigate), playback engine, scripting REPL sidebar

### Changed
- All extensions now share a unified `theme.css` with consistent Doda design tokens
- `run-all.sh` now loads all 5 extensions

### Added (Build & Tooling)
- `scripts/build-release.sh` — Full build + .xpi packaging + git tag
- `scripts/qa.sh` — 40-check QA suite for extension structure, JS syntax, branding, and theme consistency
- `design/tokens.css` — Canonical Doda design tokens (colors, typography, spacing)
- `design/theme.css` — Shared theme stylesheet distributed to all extensions

### Known Issues
- First full debug build takes 3+ hours; incremental builds are fast (seconds to minutes)
- Ollama must be running (`ollama serve`) for AI Assistant and Provenance AI analysis features
- WebExtension content scripts cannot intercept navigation events directly; URL changes in macros are handled via navigation steps
