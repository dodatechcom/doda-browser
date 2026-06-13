# Project Structure (planned)

```
Doda/
├── firefox-source/         # Mozilla Central clone (hg)
│   ├── browser/
│   │   ├── app/profile/    # Firefox preferences (doda.cfg)
│   │   ├── branding/       # Doda logo, name, metadata
│   │   ├── components/     # about: dialog, migration
│   │   ├── themes/         # Custom Doda theme
│   │   └── locales/        # en-US → en-IN adjustments
│   ├── toolkit/            # Low-level components
│   ├── dom/                # DOM engine patches
│   ├── netwerk/            # Network stack patches
│   ├── services/           # Disable sync, telemetry
│   ├── security/           # NSS, certificate handling
│   ├── third_party/        # Added: llama.cpp, didkit, libsql
│   └── mach                # Build tool
├── patches/                # Our mercurial patch queue
│   ├── rebrand.patch
│   ├── privacy-hardening.patch
│   ├── knowledge-graph.patch
│   ├── ai-assistant.patch
│   ├── did-identity.patch
│   └── provenance.patch
├── components/             # Our custom Rust components
│   ├── identity-wallet/    # DID/VC wallet
│   ├── knowledge-graph/    # Local graph DB indexer
│   ├── ai-engine/          # llama.cpp wrapper
│   └── provenance/         # Content provenance tracker
├── designs/                # UX mockups, branding assets
├── scripts/                # Build helpers, CI scripts
├── README.md
├── PLAN.md
├── CONTRIBUTING.md
└── STRUCTURE.md
```
