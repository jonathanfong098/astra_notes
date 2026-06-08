# AstraNotes — Definition of Done Checklist
## Final Audit — All Steps Complete | 2026-05-31

---

### All Tests Pass
- [x] `cmake --build build` exits 0.
- [x] `ctest --test-dir build --output-on-failure` exits 0 — **33/33 tests pass**.

### Grep Checks (Step 10)
- [x] No raw `new`/`delete` in `src/` — all grep hits confirmed as comments or string literals.
- [x] No `std::map` in `src/controller/` — single hit is in a comment only.
- [x] No `plaintext` member variable in `SecureNote.h` — hits are in comments and parameter names; private section contains only `ciphertext_` and `engine_`.
- [x] No `OPENSSL` or `AESEngine` in production headers (`src/model/`, `src/controller/`, `src/storage/`, `src/view/`) — zero hits.

### Header-File DoD
- [x] Every header in `src/` has `#pragma once` — verified by automated check (no missing headers).
- [x] Every header in `src/` has a traceability comment — verified by automated check.
- [x] Ownership semantics documented in every class (unique_ptr owned, & injected, etc.).
- [x] No raw owning pointer in any public API.

### Implementation DoD
- [x] Every `.cpp` file has traceability comments on all implemented functions.
- [x] `SecureNote::ciphertext_` is `std::vector<std::byte>` throughout.
- [x] `StorageInterface` and `EncryptionEngine` are pure abstract — no non-virtual members.
- [x] `NoteManager::notes_` is `std::unordered_map` (NFR-1 — O(1) average UUID lookup).

### Feature Completeness
- [x] FR-1 / US-01 — Note Creation (TextNote, VoiceNote, SecureNote stubs via NoteFactory).
- [x] FR-1a — Title validation: empty, whitespace-only, and >255-char titles rejected.
- [x] FR-2 / US-02 — Edit TextNote and VoiceNote title and body with atomic semantics and timestamp update. SecureNote body editing via passphrase re-lock.
- [x] FR-3 / US-03 — Delete note by UUID; SecureNote ciphertext zeroed before deallocation.
- [x] FR-4 / US-04 — JSON persistence (atomic write via tmp-then-rename); FR-4a missing file; FR-4b quarantine and per-record skip.
- [x] FR-5 / US-05 — SecureNote passphrase gate: lock() and unlock() with plaintext zeroing.
- [x] FR-6 / US-06 — Case-insensitive title substring search; empty query returns all notes.
- [x] NFR-1 — O(1) UUID lookup via unordered_map.
- [x] NFR-3 — VersionHistory populated on every body edit; title edits do not append.
- [x] NFR-4 — library-decision.md covers nlohmann/json and OpenSSL (committed before use).
- [x] SPR-1 — No plaintext in any member, log, or file; decryption buffers zeroed.
- [x] SPR-2 — EncryptionEngine and StorageInterface are pure abstract interfaces; AESEngine and FileStorage only accessed through interface references in production code.
- [x] SPR-3 — Concurrent access limitation documented in README.md.

### Documentation and Process
- [x] `planning/library-decision.md` covers nlohmann/json v3.11.3 and OpenSSL 3.6.1.
- [x] `docs/ai-reflection.md` has session entry including lab-vs-UML reconciliation note.
- [x] `planning/prompt-log.md` has session entry.
- [x] `README.md` documents build command, run command, storage path, and SPR-3 limitation.
- [x] `PROJECT_OVERVIEW.md` implementation state table updated to reflect final state.

### CLI Wiring
- [x] `NullStorage` removed from `src/main.cpp` — replaced with `FileStorage` + `AESEngine`.
- [x] Notes load from `~/.astranotes/notes.json` on startup.
- [x] Notes persist after every successful create, edit, and delete (not only on quit).
- [x] All menu commands wired: [n] new, [l] list, [v] view, [e] edit, [d] delete, [/] search, [h] help, [s] settings stub, [q] quit.

### Post-Audit Additions (2026-06-07)
- [x] TextNote body displayed in `renderNote()` via virtual `Note::getBody()` — previously invisible after editing.
- [x] VoiceNote title and audioPath editable via `[e]` command.
- [x] SecureNote title editable; body edit requires passphrase re-lock via `NoteManager::relockSecureBody()`.
- [x] Case-insensitive search wired to `[/]` CLI command; results show UUID + title pairs.
- [x] `searchByTitle()` returns `vector<pair<UUID,string>>` — actionable under duplicate titles.
- [x] Additional tests added after audit: `AddDuplicateUUID_Throws`, `SubstringMatch_ReturnsMatchingOnly`, `NoFileIODuringSearch`, `FindByUUIDReturnsNullForMissingUUID`, `EditVoiceNote_TitleAndAudioPathEditable`, `SecureNoteCiphertextMutableZeroingWorks` — total 33/33.
