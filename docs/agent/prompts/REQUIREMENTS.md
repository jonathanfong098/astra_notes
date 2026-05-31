# AstraNotes — Requirements

<!-- CHANGELOG
- Added explicit "current state" note to FR-3: remove() currently returns void; changes to Status in Step 3
- Clarified FR-2: lastModifiedAt_ belongs on Note base class, not optionally on TextNote
- Made FR-3 zeroing concrete: added std::fill pattern to match BUILD_PLAN Step 3
- Made FR-5a concrete: specified std::variant<std::string, SecureNoteError> to match BUILD_PLAN Step 6
- Added Status enum definition location: src/model/Note.h (single definition, referenced by FR-2, FR-3, FR-5a)
- Flagged FR-4 saveAll vs saveNote design tension explicitly as an open design question
- Fixed NFR-4 library-decision.md wording: "must be committed" not "is committed"
- Added NFR-3 test coverage note: VersionHistory test is not yet outlined in TEST_REFERENCE.md
- Added "not started" markers to cross-reference table for unambiguous status reading
-->

> **Purpose of this document:** The single authoritative requirement baseline for all build work.
> Source of truth is Lab 3.1 (Refined Requirement Baseline). Where Lab 3.1 and Lab 1.2 conflict,
> Lab 3.1 always wins. Every requirement ID used in traceability comments must appear here.

---

## Functional Requirements

### FR-1 — Note Creation
The system shall create a TextNote, VoiceNote, or SecureNote identified by a system-generated UUID
(RFC 4122 v4) and a user-supplied title of 1–255 non-whitespace characters. Creation is performed
exclusively through NoteFactory.

**Implementation notes:**
- UUID format: 8-4-4-4-12 hex, version bits and variant bits set per RFC 4122 v4
- Title validation: reject empty, whitespace-only, or > 255 characters
- NoteFactory is the only place UUIDs are generated and timestamps are stamped
- **Current state:** ✅ Implemented in `NoteFactory.create()` and `NoteManager.add()`

---

### FR-1a — Title Validation Failure
On title validation failure (empty, whitespace-only, or exceeding 255 characters), the system shall
return a descriptive error code and leave the note collection unchanged.

**Implementation notes:**
- Throw `std::invalid_argument` with a descriptive message
- NoteManager.add() must not be called if NoteFactory.create() throws
- The note collection must be identical before and after a failed creation
- **Current state:** ✅ Implemented. Empty and whitespace-only titles throw. 255-char test (A4) outlined.

---

### FR-2 — Edit TextNote
The system shall allow updating the title or body of an existing TextNote. Both fields obey the same
title-length rule (FR-1a). On success the last-modified timestamp shall be updated. On any failure
the note shall remain fully unchanged (no partial update).

**Implementation notes:**
- Atomic semantics: if either field update fails, neither is applied
- Editing body only must not re-validate or change the existing title
- `lastModifiedAt_` is a new field to be added to the **Note base class** (not TextNote) — all note types carry timestamps. Add getter `getLastModifiedAt()` to Note.h.
- A missing UUID returns `Status::NOT_FOUND` without throwing
- `Status` enum is defined once in `src/model/Note.h`: `enum class Status { OK, NOT_FOUND, INVALID_INPUT };`
- **Current state:** ❌ Not started. `lastModifiedAt_` field does not yet exist on Note.

---

### FR-3 — Delete Note
The system shall delete a note by UUID. Deletion shall remove the note from the in-memory collection
and release all associated resources. For SecureNote, any in-memory key material and decryption
buffers shall be zeroed before deallocation. A missing UUID shall return a failure status without throwing.

**Implementation notes:**
- `unique_ptr` destruction handles deallocation automatically (RAII)
- SecureNote zeroing before erase: `std::fill(bytes.begin(), bytes.end(), std::byte{0})` on `ciphertext_`
- This requires `getCiphertextMutable()` (returns `vector<byte>&`) added to SecureNote
- Return type must be `Status`: `Status::OK` if deleted, `Status::NOT_FOUND` if absent
- **Current state:** ⚠️ Partial. `NoteManager::remove(UUID)` exists but returns `void`. Must be changed to return `Status` in Step 3.

---

### FR-4 — JSON Persistence
The system shall persist all notes to a single local JSON file at a configurable path. On shutdown the
entire collection shall be serialized. On startup the file shall be deserialized and each note
reconstructed via NoteFactory into its correct concrete type. If two notes with the same UUID are
found in the file, the second occurrence shall be discarded and the conflict logged.

**Implementation notes:**
- File path is configurable (constructor parameter), not hardcoded
- Atomic write: write to `filePath_ + ".tmp"`, then `std::rename()` to `filePath_` — never write directly
- On load, `NoteFactory.reconstructRecord()` is used for reconstruction (currently a stub that throws)
- UUID collision on load: log to `std::cerr` and discard the second record
- **Open design question:** `StorageInterface` currently declares `saveNote(const Note&)`. FR-4 requires serializing the entire collection atomically in one write. Implementations must resolve this: either add `saveAll(const unordered_map<UUID, unique_ptr<Note>>&)` to StorageInterface, or have `NoteManager::persistAll()` loop and call `saveNote` then write once. Resolve this decision in `FileStorage` and update `StorageInterface` if needed. Document the choice in `planning/library-decision.md`.
- **Current state:** ❌ Not started. FileStorage does not exist.

---

### FR-4a — Missing Storage File
If the storage file is absent at startup, the system shall start with an empty collection and create a
new file on the next save. A missing file is not an error.

**Current state:** ❌ Not started (FileStorage not yet implemented).

---

### FR-4b — Corrupt Storage File
If the storage file is present but unreadable or structurally corrupt, the system shall rename it to
`<filename>.quarantine.<timestamp>`, log the event, and start with an empty collection. Individual
malformed note records within an otherwise valid file shall be skipped individually; all valid records
shall still load.

**Implementation notes:**
- Quarantine filename must include a Unix timestamp to avoid overwriting previous quarantine files
- Per-record skip: a single bad JSON object does not quarantine the whole file
- All healthy records in a partially corrupt file must load successfully — do not stop at the first bad record
- **Current state:** ❌ Not started (FileStorage not yet implemented).

---

### FR-5 — SecureNote Passphrase Gate
The system shall allow reading the decrypted content of a SecureNote only when supplied with the
correct passphrase. The passphrase shall be at least 8 characters. Plaintext shall not be retained in
any data member, log entry, or temporary buffer after the access operation completes. The decryption
buffer shall be zeroed immediately after use.

**Implementation notes:**
- Minimum passphrase length: 8 characters (checked by `.size()`, not `strlen()`)
- Passphrase is treated as an opaque byte sequence (`std::string`); supports null bytes
- Plaintext is a local variable only; it must not be stored in any SecureNote member
- Zero the decryption buffer: `std::fill(buf.begin(), buf.end(), '\0')` before it goes out of scope
- **Current state:** ❌ Not started. SecureNote has no `lock()`/`unlock()` methods.

---

### FR-5a — Passphrase Failure Error Codes
On passphrase failure (wrong, empty, or below minimum length), the system shall return a typed error
code distinguishing wrong-passphrase from invalid-input. No partial plaintext shall be accessible.

**Implementation notes:**
- Return type for `unlock()`: `std::variant<std::string, SecureNoteError>`
- `SecureNoteError` enum: `enum class SecureNoteError { WRONG_PASSPHRASE, INVALID_INPUT };`
- Empty passphrase → `INVALID_INPUT` (do not call `engine_.decrypt`)
- Passphrase < 8 chars → `INVALID_INPUT`
- Wrong passphrase (engine throws or returns failure) → `WRONG_PASSPHRASE`
- **Current state:** ❌ Not started.

---

### FR-6 — Title Substring Search
The system shall search the in-memory note collection by title substring and return all matching notes.
Search shall be case-insensitive. No file I/O shall occur during search. An empty query shall return
the full collection, not an error.

**Implementation notes:**
- ✅ Implemented in `NoteManager::searchByTitle` using `std::transform`/`std::tolower`
- Empty query: `std::string::find("")` returns 0 for any string — every note matches (correct per FR-6)
- **Gap:** Empty-query behavior is not yet tested — test D4 (`SearchByTitle.EmptyQuery_ReturnsAllNotes`) is missing. This is the only blocker for closing PR #1.
- Return type is `std::vector<std::string>` (titles only)
- **Current state:** ⚠️ Implementation complete; D4 test missing.

---

## Non-Functional Requirements

### NFR-1 — O(1) UUID Lookup
UUID-keyed note lookup shall complete in O(1) average time using an `unordered_map` or equivalent
hash structure. The data structure shall be the sole authoritative in-memory store.

**Implementation notes:**
- ✅ Implemented: `std::unordered_map<UUID, std::unique_ptr<Note>>` in NoteManager
- `std::map` is explicitly prohibited for the notes collection (gives O(log n))
- Side effect: iteration order is now non-deterministic (hash order). A backlog item exists to decide whether stable display order is needed before Sprint 3.
- **Current state:** ✅ Complete.

---

### NFR-2 — No Undefined Behavior
The system shall not crash or invoke undefined behavior (as detected by AddressSanitizer in debug
builds) under any single-operation input, including empty strings, maximum-length strings, and
missing UUIDs.

**Implementation notes:**
- All boundary inputs must have corresponding test cases
- ASan clean is the measurable standard (not just "no crash")
- **Current state:** Partially covered by existing tests; boundary cases added per build step.

---

### NFR-3 — Smart Pointer Ownership
All owning pointers shall use `unique_ptr`. `shared_ptr` is permitted only where shared ownership is
documented with an explicit rationale comment in the relevant header. Raw owning pointers are
prohibited.

**Implementation notes:**
- **Current state:** ✅ Enforced throughout existing code. Verified by grep (no raw new/delete in src/).
- Test coverage: VersionHistory population test is not yet outlined in TEST_REFERENCE.md — add in Step 9.

---

### NFR-4 — External Library Justification
Any third-party library adopted shall be documented in `planning/library-decision.md` with: C++17
compatibility evidence, GTest linkage test result, license classification, and supply-chain risk
assessment. The file must be committed before the library is used.

**Current libraries:**
- nlohmann/json v3.11.3 — justification must be in `planning/library-decision.md` (verify this file is committed)
- OpenSSL libcrypto — justification required and must be committed before Step 7 (AESEngine implementation)
- **Current state:** ⚠️ nlohmann/json may be justified; verify `planning/library-decision.md` exists and is committed. OpenSSL justification not yet written.

---

## Security / Privacy Requirements

### SPR-1 — No Plaintext on Disk
No SecureNote plaintext shall appear in any JSON file, log file, core dump, or temporary file at any
time. SecureNote JSON records shall contain only: ciphertext, UUID, title, created timestamp, and
last-modified timestamp.

**Implementation notes:**
- `SecureNote::ciphertext_` is always `std::vector<std::byte>` — never `std::string`
- FileStorage must serialize the ciphertext bytes (e.g., as hex string) directly
- Verify with grep: no SecureNote plaintext field in any `.json` output
- **Current state:** ❌ FileStorage not yet implemented. Interface constraint enforced by type system.

---

### SPR-2 — Injectable Encryption Interface
The `EncryptionEngine` shall be defined as an injectable interface (pure abstract class). No
SecureNote, NoteManager, or FileStorage class shall hold a concrete crypto dependency in its public
API. Unit tests shall be able to substitute a mock EncryptionEngine without linking a real crypto
library.

**Implementation notes:**
- ✅ Implemented: `EncryptionEngine` is a pure abstract class in `encryption/EncryptionEngine.h`
- `SecureNote` holds `EncryptionEngine&` (injected reference, not owned)
- Tests use a `MockEncryptionEngine` inline in test files
- **Current state:** ✅ Interface complete. Concrete implementation (AESEngine) deferred to Step 7.

---

### SPR-3 — No Concurrent Access
Concurrent access to the same storage file by two application instances is undefined behavior in v1.0.
The README shall document this limitation explicitly. Future versions may introduce file locking.

**Current state:** ❌ README does not yet document this. Add in Step 10 final audit.

---

## Requirements → User Story Cross-Reference

| Requirement ID | User Story | Sprint Target | Status |
|---|---|---|---|
| FR-1, FR-1a | US-01 (Note Creation) | Sprint 1 | ✅ Done |
| FR-2 | US-02 (Edit TextNote) | Sprint 2 | ❌ Not started |
| FR-3 | US-03 (Delete Note) | Sprint 2 | ⚠️ remove() exists, returns void |
| FR-4, FR-4a, FR-4b | US-04 (Persistence) | Sprint 2 | ❌ Not started |
| FR-5, FR-5a | US-05 (SecureNote Gate) | Sprint 2–3 | ❌ Not started |
| FR-6 | US-06 (Search) | Sprint 3 | ⚠️ Impl done, D4 test missing |
| NFR-1 | — | Sprint 1 | ✅ Done |
| NFR-2 | — | All sprints | ⚠️ Partial coverage |
| NFR-3 | — | All sprints | ✅ Enforced; Step 9 test pending |
| NFR-4 | — | Sprint 0 | ⚠️ nlohmann done; OpenSSL pending |
| SPR-1 | US-05 | Sprint 3 | ❌ Not started |
| SPR-2 | US-05 | Sprint 0 | ✅ Interface complete |
| SPR-3 | — | v1.0 README | ❌ README note not yet added |
