# AstraNotes — Project Overview

<!-- CHANGELOG
- Added "Current Sprint and Out-of-Scope Constraints" section (was in CLAUDE.md only; Claude Code needs it here)
- Added "Open Items and Known Technical Debt" section (PR #1 open, D4 test missing, display-order issue)
- Added planning/library-decision.md to repository structure (required by NFR-4, was missing)
- Added PROMPT.md to repository structure (exists in repo root, was missing)
- Expanded planning/ subdirectory listing to show all Sprint Zero planning files
- Added note to implementation table: Note.h does not yet have lastModifiedAt_ (added in Step 2)
- Clarified test baseline note: search tests are in same file as creation tests
-->

> **Purpose of this document:** This is the first file Claude Code reads at the start of every session.
> It establishes what the project is, what exists today, and the non-negotiable rules that govern every
> decision. Nothing in this file is aspirational — it reflects the actual current state of the repository.

---

## What AstraNotes Is

AstraNotes is a local-first, single-user, privacy-first C++17 note-taking application built as a
semester-long course project (CSEN 296B-2, Prof. Paddu Melanahalli). It is a command-line application
that runs on macOS, Linux, and Windows. There are no network dependencies. All data is stored locally
at `~/.astranotes/notes.json`.

The application supports three note types: plain text (`TextNote`), audio-backed (`VoiceNote`), and
encrypted (`SecureNote`). It is designed to accommodate new note types without modifying existing code.

---

## Tech Stack (Non-Negotiable)

| Concern | Choice | Constraint |
|---|---|---|
| Language | C++17 | No C++20 features. Ever. |
| Architecture | MVC | CLIView = View, NoteManager = Controller, Note hierarchy = Model |
| Ownership | RAII via `unique_ptr` | No raw `new`/`delete`. No raw owning pointers in public API. `shared_ptr` only with documented rationale in the header. |
| Build | CMake 3.14+ | FetchContent for all dependencies |
| Testing | Google Test v1.14.0 | `cmake --build` and `ctest` must both exit 0 before any step is complete |
| JSON | nlohmann/json v3.11.3 | Header-only, fetched via FetchContent |
| Encryption | OpenSSL libcrypto | AES-256-GCM. Currently declared in CMake but not yet linked to any target. |
| Interfaces | Pure abstract classes | `StorageInterface` and `EncryptionEngine` are pure virtual. No concrete dependency in public APIs. |

---

## Repository Structure

```
AstraNotes/
├── CMakeLists.txt
├── CLAUDE.md                          # Claude Code session rules (hard constraints)
├── PROMPT.md                          # Meta-prompt used to generate the Week 6 Claude Code prompt
├── README.md
├── .gitignore
├── docs/
│   ├── agent/                         # ← YOU ARE HERE (all AI agent documents)
│   │   ├── PROJECT_OVERVIEW.md
│   │   ├── REQUIREMENTS.md
│   │   ├── ARCHITECTURE.md
│   │   ├── BUILD_PLAN.md
│   │   ├── TEST_REFERENCE.md
│   │   └── prompts/
│   │       ├── EVAL_PROJECT_OVERVIEW.md
│   │       ├── EVAL_REQUIREMENTS.md
│   │       ├── EVAL_ARCHITECTURE.md
│   │       ├── EVAL_BUILD_PLAN.md
│   │       └── EVAL_TEST_REFERENCE.md
│   ├── ai-reflection.md
│   └── dod-checklist.md
├── planning/
│   ├── prompt-log.md
│   ├── library-decision.md            # NFR-4: justification for nlohmann/json (required before OpenSSL too)
│   ├── requirements.md                # Sprint Zero planning artifact
│   ├── user-stories.md                # Sprint Zero planning artifact
│   ├── backlog.md                     # Sprint Zero planning artifact
│   ├── sprint-zero-plan.md            # Sprint Zero planning artifact
│   ├── traceability.md                # Sprint Zero planning artifact
│   └── risk-log.md                    # Sprint Zero planning artifact
├── src/
│   ├── main.cpp
│   ├── controller/
│   │   ├── NoteFactory.h / .cpp
│   │   └── NoteManager.h / .cpp
│   ├── encryption/
│   │   └── EncryptionEngine.h         # pure abstract interface
│   ├── model/
│   │   ├── Note.h / .cpp              # abstract base
│   │   ├── TextNote.h / .cpp
│   │   ├── VoiceNote.h / .cpp
│   │   ├── SecureNote.h / .cpp
│   │   ├── VersionHistory.h / .cpp
│   │   └── VersionEntry.h
│   ├── storage/
│   │   └── StorageInterface.h         # pure abstract interface
│   └── view/
│       └── CLIView.h / .cpp
└── test/
    ├── CMakeLists.txt
    └── test_note_creation.cpp         # contains both NoteCreation and SearchByTitle tests
```

---

## Current Sprint and Out-of-Scope Constraints

**Active sprint:** Sprint Zero complete + Note Creation slice (US-01 / FR-1) complete.
**Next action:** Execute BUILD_PLAN.md starting at Step 1.

**Explicitly out of scope until the relevant BUILD_PLAN step:**

| Feature | Out-of-scope until |
|---|---|
| FileStorage body (JSON read/write) | Step 4 |
| AESEngine concrete crypto | Step 7 |
| SecureNote passphrase gate (lock/unlock) | Step 6 |
| TextNote edit (editTitle / editBody) | Step 2 |
| Note delete (with Status return) | Step 3 |
| VersionHistory population (addEntry) | Step 9 |
| VoiceNote audio handling | Not in current plan |

Do not implement any of the above until their designated step is approved.

---

## Current Implementation State

This table is the authoritative record of what is done, stubbed, or not started.
**Last updated: Step 10 final audit (2026-05-31). All steps complete.**

| Component | File(s) | Status | Notes |
|---|---|---|---|
| Note abstract base | `model/Note.h/.cpp` | ✅ Complete | UUID, title, createdAt, lastModifiedAt, VersionHistory composition; reconstruction constructor; setTitle, refreshLastModified, non-const getVersionHistory |
| TextNote | `model/TextNote.h/.cpp` | ✅ Complete | body field, getBody/setBody; reconstruction constructor |
| VoiceNote | `model/VoiceNote.h/.cpp` | ⚠️ Stub | audioPath always empty; audio handling deferred; reconstruction constructor present |
| SecureNote | `model/SecureNote.h/.cpp` | ✅ Complete | ciphertext_ is `vector<byte>`; lock/unlock with passphrase gate and buffer zeroing; getCiphertextMutable for delete zeroing; reconstruction constructor |
| VersionHistory | `model/VersionHistory.h/.cpp` | ✅ Complete | addEntry() implemented; populated by NoteManager::editBody |
| VersionEntry | `model/VersionEntry.h` | ✅ Complete | struct only; no behavior |
| NoteFactory | `controller/NoteFactory.h/.cpp` | ✅ Complete | RFC 4122 v4 UUID; title validation (empty, whitespace, >255 chars); text/voice creation; reconstructRecord fully implemented for all three types |
| NoteManager | `controller/NoteManager.h/.cpp` | ✅ Complete | add, remove (Status), findByUUID, searchByTitle (case-insensitive), editTitle, editBody (with VersionHistory), persistAll, loadAll, getNotes |
| StorageInterface | `storage/StorageInterface.h` | ✅ Complete | Pure abstract; saveNote + saveAll (default loop) + loadNotes |
| FileStorage | `storage/FileStorage.h/.cpp` | ✅ Complete | Atomic tmp-then-rename write; FR-4a missing file; FR-4b quarantine + per-record skip + UUID collision discard; uses NoteFactory::reconstructRecord |
| EncryptionEngine | `encryption/EncryptionEngine.h` | ✅ Complete | Pure abstract; encrypt + decrypt |
| AESEngine | `encryption/AESEngine.h/.cpp` | ✅ Complete | AES-256-GCM via OpenSSL libcrypto; PBKDF2-SHA256 key derivation; SALT\|IV\|TAG\|CIPHERTEXT wire format; key and plaintext buffers zeroed |
| CLIView | `view/CLIView.h/.cpp` | ✅ Complete | renderNoteList, renderNote, promptInput |
| main.cpp | `src/main.cpp` | ✅ Complete | FileStorage + AESEngine wired; loadAll on startup, persistAll on quit; [n]/[l]/[v]/[e]/[d]/[s]/[q] menu |
| NullStorage | inline in `test/` only | ✅ Complete | No-op StorageInterface for unit tests; removed from main.cpp in Step 8 |

### What the Tests Cover

All 27 tests are in `test/test_note_creation.cpp`.

| Suite | Count | Requirements |
|---|---|---|
| NoteCreation (A1–A4) | 4 | FR-1, FR-1a |
| SearchByTitle (D2, D3, D4) | 3 | FR-6 |
| EditTextNote (A8–A10) | 3 | FR-2 |
| DeleteNote (A11–A14) | 4 | FR-3, SPR-1 |
| FileStorageTest (C1–C6, B6) | 7 | FR-4, FR-4a, FR-4b, SPR-1 |
| VersionHistory (E1) | 1 | NFR-3 |
| SecureNote (B1–B5) | 5 | FR-5, FR-5a, SPR-1, SPR-2 |

**Total: 27/27 passing.**

---

## Open Items and Known Technical Debt

These items are known gaps that must be resolved at the steps indicated. They are not defects —
they are tracked work. Do not silently fix them outside the designated step.

| Item | Impact | Resolves In |
|---|---|---|
| PR #1 (`feature/search-improvement`) is open on GitHub | searchByTitle is implemented in local code but the PR has not been merged; empty-query test (D4) is missing | Step 1 |
| Test D4 missing: `SearchByTitle.EmptyQuery_ReturnsAllNotes` | FR-6 requires "empty query returns full collection" — no test covers this | Step 1 |
| `NoteManager::remove()` returns `void` | FR-3 requires a `Status` return distinguishing "deleted" from "not found" | Step 3 |
| `Note` has no `lastModifiedAt_` field | FR-2 (edit) requires timestamp update on edit | Step 2 |
| Display order is non-deterministic | Switching from `std::map` to `unordered_map` (PR #2) removed sorted iteration order; no requirement specifies display order yet, but it is a known behavioral change | Step 9 or backlog |
| `planning/library-decision.md` needs OpenSSL justification | NFR-4 requires justification before any library is used; OpenSSL will be linked in Step 7 | Step 7 |

---

## Build and Run

```bash
# Configure and build
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build

# Run tests
ctest --test-dir build --output-on-failure

# Run the application
./build/astra_notes
```

---

## Hard Rules Claude Code Must Always Follow

These rules are also in `CLAUDE.md`. They are repeated here because they govern every build step.

1. **C++17 only.** No C++20 features anywhere.
2. **`unique_ptr` for all ownership.** `shared_ptr` requires a documented rationale comment in the header.
3. **No raw `new` or `delete`.** No raw owning pointers in any public API.
4. **MVC separation.** CLIView never accesses Note instances directly. NoteManager never does I/O directly.
5. **`ctest` must exit 0** before any build step is marked complete. No exceptions.
6. **Traceability comments required** on every header and every implemented function:
   `// Traceability: FR-1 (refined) | UML: NoteFactory.create`
7. **SecureNote:** `ciphertext_` is always `std::vector<std::byte>`. Never `std::string` or `char*`. No plaintext in any member, log, or file.
8. **`EncryptionEngine` and `StorageInterface` are pure abstract.** No concrete crypto or filesystem type appears in any public API.
9. **Do not invent classes.** Every class must appear in `ARCHITECTURE.md`. If a new class is genuinely needed, stop and document it before implementing.

---

## Traceability Chain

```
Project Charter → Initial Requirements (Lab 1.2)
    → Refined Requirements (Lab 3.1)           ← REQUIREMENTS.md is derived from this
        → User Stories + Acceptance Criteria (Lab 2.2)
            → UML Design Package (Lab 4.2)     ← ARCHITECTURE.md is derived from this
                → Codebase (Week 6+)
                    → Tests (Lab 7.2)          ← TEST_REFERENCE.md is derived from this
                        → Build Plan           ← BUILD_PLAN.md sequences all of the above
```
