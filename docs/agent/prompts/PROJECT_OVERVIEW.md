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
**Update this table whenever a build step completes.**

| Component | File(s) | Status | Notes |
|---|---|---|---|
| Note abstract base | `model/Note.h/.cpp` | ✅ Complete | UUID, title, createdAt timestamps, VersionHistory composition. Note: `lastModifiedAt_` field not yet added — added in Step 2. |
| TextNote | `model/TextNote.h/.cpp` | ✅ Complete | body field, getBody/setBody |
| VoiceNote | `model/VoiceNote.h/.cpp` | ⚠️ Stub | audioPath always empty; no audio handling |
| SecureNote | `model/SecureNote.h/.cpp` | ⚠️ Stub | ciphertext_ is `vector<byte>`; no lock/unlock yet |
| VersionHistory | `model/VersionHistory.h/.cpp` | ⚠️ Stub | entries_ always empty; addEntry not yet implemented |
| VersionEntry | `model/VersionEntry.h` | ✅ Complete | struct only; no behavior |
| NoteFactory | `controller/NoteFactory.h/.cpp` | ✅ Complete | RFC 4122 v4 UUID, title validation, text/voice creation. reconstructRecord is a stub (throws). |
| NoteManager | `controller/NoteManager.h/.cpp` | ✅ Complete | add, remove (void), findByUUID, searchByTitle (case-insensitive, fully implemented). Note: remove() returns void now; changes to Status in Step 3. |
| StorageInterface | `storage/StorageInterface.h` | ✅ Complete | Pure abstract; saveNote + loadNotes |
| FileStorage | *(not yet created)* | ❌ Not started | JSON round-trip, quarantine logic — Step 4 |
| EncryptionEngine | `encryption/EncryptionEngine.h` | ✅ Complete | Pure abstract; encrypt + decrypt |
| AESEngine | *(not yet created)* | ❌ Not started | AES-256-GCM via OpenSSL libcrypto — Step 7 |
| CLIView | `view/CLIView.h/.cpp` | ✅ Complete | renderNoteList, renderNote, promptInput |
| main.cpp | `src/main.cpp` | ✅ Complete | n/l/s/q menu; settings stub; uses NullStorage |
| NullStorage | inline in `main.cpp` and `test/` | ✅ Complete | No-op StorageInterface for tests and dev |

### What the Tests Currently Cover

All tests are in `test/test_note_creation.cpp`.

| Test | Suite | Status |
|---|---|---|
| `NoteCreation.HappyPath_TextNoteCreatedAndStored` | Note Creation | ✅ Passing |
| `NoteCreation.EmptyTitle_ThrowsInvalidArgument` | Note Creation | ✅ Passing |
| `NoteCreation.TwoNotesWithSameTitle_ReceiveDistinctUUIDs` | Note Creation | ✅ Passing |
| `SearchByTitle.CaseInsensitive_UpperQueryMatchesLowerTitle` | Search | ✅ Passing |
| `SearchByTitle.NoMatch_ReturnsEmptyVector` | Search | ✅ Passing |

**Total: 5/5 passing. This is the Step 0 baseline.**

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
