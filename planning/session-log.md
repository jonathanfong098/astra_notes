# AstraNotes — Session Log
**Date:** 2026-05-25
**Branch at session start:** main
**Working directory:** /Users/jonathanfong/Desktop/csen296/astra_notes

---

## Step 1: Execute PROMPT.md

PROMPT.md was read and executed. Its role: act as a senior software engineer and produce a single, self-contained Claude Code prompt to bootstrap the AstraNotes project for Week 6.

The generated Claude Code prompt covered 10 sections:
1. Context (C++17, MVC, CMake, Google Test, scope)
2. Repository scaffold (folder tree + empty files)
3. Build environment (CMakeLists.txt with FetchContent for GTest + nlohmann/json)
4. Interface headers (Note hierarchy, VersionHistory, StorageInterface, EncryptionEngine, NoteFactory, NoteManager, CLIView)
5. Sprint 1 slice: Note Creation (US-01 / FR-1) with acceptance criteria
6. CLI shell (main.cpp with [n]/[l]/[s]/[q] loop)
7. Traceability instructions (comment format for every header and function)
8. Definition of Done checklist
9. Prompt log + AI reflection (including lab-vs-UML reconciliation note)
10. Explicit out-of-scope list

---

## Step 2: Project Created

All 30 files scaffolded and implemented across:

```
astra_notes/
├── CMakeLists.txt
├── .gitignore
├── README.md
├── planning/
│   └── prompt-log.md
├── docs/
│   ├── ai-reflection.md
│   └── dod-checklist.md
├── src/
│   ├── model/
│   │   ├── Note.h / Note.cpp           (abstract base, UUID typedef, VersionHistory owner)
│   │   ├── TextNote.h / TextNote.cpp   (concrete; getType() = "text")
│   │   ├── VoiceNote.h / VoiceNote.cpp (stub; getType() = "voice")
│   │   ├── SecureNote.h / SecureNote.cpp (stub; ciphertext_ = std::vector<std::byte>)
│   │   ├── VersionHistory.h / .cpp     (stub; entries_ always empty)
│   │   └── VersionEntry.h              (plain struct)
│   ├── controller/
│   │   ├── NoteFactory.h / .cpp        (create() with pseudo-UUID gen; reconstructRecord() stub)
│   │   ├── NoteManager.h / .cpp        (add, remove, findByUUID, searchByTitle stub, getNotes)
│   ├── storage/
│   │   └── StorageInterface.h          (pure abstract; saveNote, loadNotes)
│   ├── encryption/
│   │   └── EncryptionEngine.h          (pure abstract; encrypt, decrypt)
│   ├── view/
│   │   ├── CLIView.h / .cpp            (renderNoteList, renderNote, promptInput)
│   └── main.cpp                        ([n]/[l]/[s]/[q] loop; NullStorage inline)
└── test/
    ├── CMakeLists.txt
    └── test_note_creation.cpp          (3 GTest cases)
```

**Key constraints met:**
- C++17 only (no C++20 features)
- `std::unique_ptr` for all ownership; no `new`/`delete`
- `SecureNote::ciphertext_` is `std::vector<std::byte>` (never std::string)
- `StorageInterface` and `EncryptionEngine` are pure abstract interfaces
- Traceability comments on every header and implemented function
- Lab-vs-UML reconciliation: Settings implemented as `[s]` stub in main.cpp; no SettingsManager invented

---

## Step 3: Build and Test (initial)

```
cmake -B build -DCMAKE_BUILD_TYPE=Debug   → exit 0
cmake --build build                        → exit 0 (all 8 sources compiled)
ctest --test-dir build --output-on-failure → exit 0
```

**Tests passed (3/3):**
```
1/3  NoteCreation.HappyPath_TextNoteCreatedAndStored     Passed
2/3  NoteCreation.EmptyTitle_ThrowsInvalidArgument        Passed
3/3  NoteCreation.TwoNotesWithSameTitle_ReceiveDistinctUUIDs  Passed
```

---

## Step 4: Push to GitHub

```bash
git init
git add <all 30 project files>
git commit -m "Initial commit: Sprint Zero + Note Creation slice (US-01 / FR-1)"
git remote add origin git@github.com:jonathanfong098/astra_notes.git
git branch -M main
git push -u origin main
```

Commit: `5eca40e`
Remote: https://github.com/jonathanfong098/astra_notes

---

## Step 5: Branch 1 — feature/search-improvement

**Goal:** Implement FR-6 — case-insensitive `searchByTitle` in NoteManager.

```bash
git checkout -b feature/search-improvement
```

### Changes made

**src/controller/NoteManager.cpp:**
- Added `#include <algorithm>` and `#include <cctype>`
- Replaced stub `searchByTitle` (returned `{}`) with real implementation:
  - Lambda `toLower` converts a string to lowercase via `std::transform` + `std::tolower`
  - Iterates all notes; matches if `toLower(title).find(toLower(query)) != npos`
  - Returns vector of matching titles

**test/test_note_creation.cpp:**
- Added `SearchByTitle.CaseInsensitive_UpperQueryMatchesLowerTitle`:
  - Creates notes "alpha notes" and "Beta reminder"
  - Queries "ALPHA" → expects exactly 1 result: "alpha notes"
- Added `SearchByTitle.NoMatch_ReturnsEmptyVector`:
  - Creates note "Meeting notes"
  - Queries "zzz" → expects empty vector

### Build & test issue encountered

First `cmake --build` failed:
```
fatal error: 'string' file not found
```
**Cause:** Stale CMake build cache with an outdated Xcode SDK path (MacOSX26.4.sdk).
**Resolution:** Deleted `build/` and ran a clean `cmake -B build` configure.

### Build & test result

```
cmake --build build  → exit 0
ctest                → 5/5 passed
```

```
1/5  NoteCreation.HappyPath_TextNoteCreatedAndStored            Passed
2/5  NoteCreation.EmptyTitle_ThrowsInvalidArgument               Passed
3/5  NoteCreation.TwoNotesWithSameTitle_ReceiveDistinctUUIDs     Passed
4/5  SearchByTitle.CaseInsensitive_UpperQueryMatchesLowerTitle    Passed
5/5  SearchByTitle.NoMatch_ReturnsEmptyVector                     Passed
```

### Commit & push

```bash
git add src/controller/NoteManager.cpp test/test_note_creation.cpp
git commit -m "feat: make searchByTitle case-insensitive per FR-6"
git push -u origin feature/search-improvement
```

Commit: `541e4b2`

---

## Step 6: Branch 2 — fix/test-cleanup

**Goal:** Fix NFR-1 — replace `std::map` with `std::unordered_map` in NoteManager for O(1) average UUID lookup.

```bash
git checkout main
git checkout -b fix/test-cleanup
```

### Changes made

**src/controller/NoteManager.h:**
- Replaced `#include <map>` with `#include <unordered_map>`
- Changed `notes_` field: `std::map<UUID, std::unique_ptr<Note>>` → `std::unordered_map<UUID, std::unique_ptr<Note>>`
- Updated `getNotes()` return type to match
- Added NFR-1 traceability comment on the `notes_` field

**src/controller/NoteManager.cpp:**
- Updated `getNotes()` return type to `const std::unordered_map<UUID, std::unique_ptr<Note>>&`

**No other logic changes needed:** `unordered_map` and `map` share the same `emplace`, `find`, `erase`, and `count` interface used in this codebase. `CLIView.cpp` was verified to have no map-specific API usage.

### Build & test result

```
cmake --build build  → exit 0
ctest                → 3/3 passed
```

```
1/3  NoteCreation.HappyPath_TextNoteCreatedAndStored     Passed
2/3  NoteCreation.EmptyTitle_ThrowsInvalidArgument        Passed
3/3  NoteCreation.TwoNotesWithSameTitle_ReceiveDistinctUUIDs  Passed
```

(Search tests not present on this branch — they live on feature/search-improvement only.)

### Commit & push

```bash
git add src/controller/NoteManager.h src/controller/NoteManager.cpp
git commit -m "fix: replace std::map with unordered_map in NoteManager per NFR-1"
git push -u origin fix/test-cleanup
```

Commit: `4193811`

---

## Final State

| Branch                     | Commit    | Tests  | Pushed |
|----------------------------|-----------|--------|--------|
| main                       | 5eca40e   | 3/3    | yes    |
| feature/search-improvement | 541e4b2   | 5/5    | yes    |
| fix/test-cleanup           | 4193811   | 3/3    | yes    |

No pull requests opened. No branches merged. Stopped after pushing both branches.
