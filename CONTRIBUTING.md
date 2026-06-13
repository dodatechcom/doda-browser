# Contributing to Doda

## Prerequisites

- Rust, C++, Mercurial, mozbuild (see PLAN.md Phase 0)
- Read [Mozilla Coding Style](https://firefox-source-docs.mozilla.org/code-review/coding-style.html)

## Workflow

1. Create a patch branch from `mozilla-central` upstream
2. Apply Doda patches via `hg queue` or git-cinnabar
3. Build & test: `./mach build && ./mach test`
4. Submit PR via GitHub (git-cinnabar mirrors)

## Areas to Contribute

- **Rust components**: Knowledge graph, identity wallet, AI inference
- **C++/Gecko**: Privacy patches, provenance hooks
- **Frontend (XUL/HTML/JS)**: Sidebar UI, settings, onboarding
- **Branding/Design**: Icons, themes, UX

## Code of Conduct

Be respectful. This is a user-first, privacy-first project.
