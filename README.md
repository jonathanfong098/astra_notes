# AstraNotes

A C++17 CLI note-taking application built with MVC architecture. The Model layer comprises the `Note` hierarchy (`TextNote`, `VoiceNote`, `SecureNote`), the Controller layer is `NoteManager` and `NoteFactory`, and the View layer is `CLIView`. Storage and encryption are provided via pure abstract interfaces.

## Prerequisites

- CMake 3.14 or higher
- C++17 compiler (Apple Clang, GCC 9+, or MSVC 2019+)
- OpenSSL 3.x (required for SecureNote AES-256-GCM encryption)
  - macOS (Homebrew): `brew install openssl@3`
  - Ubuntu/Debian: `sudo apt-get install libssl-dev`
  - Windows: `choco install openssl` or download from https://slproweb.com/products/Win32OpenSSL.html
- Google Test and nlohmann/json are fetched automatically by CMake via FetchContent — no manual install needed

## Features

| Feature | Status | Requirement |
|---|---|---|
| Create TextNote, VoiceNote, SecureNote | ✅ Complete | FR-1 |
| Title validation (empty, whitespace, >255 chars) | ✅ Complete | FR-1a |
| Edit note title and body (atomic semantics) | ✅ Complete | FR-2 |
| Delete note by UUID with confirmation | ✅ Complete | FR-3 |
| JSON persistence with atomic write | ✅ Complete | FR-4 |
| Corrupt file quarantine and recovery | ✅ Complete | FR-4b |
| SecureNote AES-256-GCM passphrase gate | ✅ Complete | FR-5 |
| Case-insensitive title substring search | ✅ Complete | FR-6 |
| Version history per note (body edits) | ✅ Complete | NFR-3 |

**Build:**
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Debug && cmake --build build && ctest --test-dir build --output-on-failure
```

**Run:**
```bash
./build/astra_notes
```
Notes are stored at `~/.astranotes/notes.json`.

## Commands

| Command | Description |
|---|---|
| `n` | New note — prompts for type (text/voice/secure), title, and body |
| `l` | List all notes (UUID, type, title) |
| `v` | View note details — prompts for UUID; unlocks SecureNote with passphrase |
| `e` | Edit note — prompts for UUID, field (title/body), new value |
| `d` | Delete note — prompts for UUID and confirmation |
| `/` | Search notes by title substring (case-insensitive) |
| `h` | Help — show command descriptions |
| `s` | Settings (stub — not yet implemented) |
| `q` | Quit — saves all notes and exits |

## Repository Structure

| Path | Contents |
|---|---|
| `src/` | C++17 source code (model, controller, storage, encryption, view) |
| `test/` | Google Test suite (33 tests covering all requirements) |
| `docs/agent/prompts/` | Agent context documents: requirements, architecture, build plan, test reference |
| `docs/` | Project documentation: AI reflection log, DoD checklist, test results |
| `planning/` | Sprint Zero planning artifacts: backlog, risk log, traceability, library decisions |

## Lab Submissions and Source Documents

The `docs/agent/prompts/` directory contains the authoritative agent context documents
synthesized and refined from the weekly lab submissions over the course of the project.

| Lab | Content | Synthesized Into |
|---|---|---|
| Lab 1.2 — Initial Requirements | First requirement set | `docs/agent/prompts/REQUIREMENTS.md` |
| Lab 2.1 — Definition of Done | DoD checklist | `docs/dod-checklist.md` |
| Lab 2.2 — Backlog and Sprint Zero Plan | User stories, backlog, sprint plan | `docs/agent/prompts/BUILD_PLAN.md` |
| Lab 3.1 — Refined Requirement Baseline | Authoritative refined requirements | `docs/agent/prompts/REQUIREMENTS.md` |
| Lab 4.1 — UML Design Package | All five UML diagrams | `docs/agent/prompts/ARCHITECTURE.md` |
| Lab 7.2 — Testing Strategy and First Test Set | Test strategy and test outlines | `docs/agent/prompts/TEST_REFERENCE.md` |
| Week 8.1 — Git Workflow | Branch workflow, PR review, merge decisions | `docs/ai-reflection.md` |

**Known limitations (v1.0):**

> **SPR-3:** Concurrent access to the same storage file by two application instances is undefined behavior in v1.0. Only one instance of AstraNotes should run against a given storage file at a time. File locking is not implemented.
