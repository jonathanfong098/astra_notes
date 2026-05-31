# AstraNotes — Build Plan

<!-- CHANGELOG
- Assigned A4 (255-char title test) to Step 1 alongside D4 (was unassigned)
- Added Note.h and Note.cpp to Step 2 files list (Status enum addition was missing)
- Added A14 (SecureNote ciphertext zeroed after delete) to Step 3 test list and gate
- Added SecureNoteError location (SecureNote.h) to Step 6 files list
- Fixed Step 7 CMake variable: OpenSSL_FOUND → OPENSSL_FOUND
- Labeled manual verification items in Step 8 gates explicitly as [manual]
- Added Note.h and Note.cpp to Step 9 files list (non-const getVersionHistory() was missing)
- Assigned test ID E1 to VersionHistory population test in Step 9
- Added E1 to TEST_REFERENCE tracking note
- Clarified Step 1 dependency rationale (workflow gate, not technical dependency)
- Clarified Step 6 dependency rationale (workflow serialization, not technical dependency)
- Added note to Step 3 gate: A14 is the automated check for SecureNote zeroing
-->

> **Purpose of this document:** The sequenced, step-by-step instruction set for Claude Code.
> Each step is a single logical feature or sub-feature. Claude Code executes one step,
> then **STOPS**. Jonathan verifies the output against the DoD gate before approving the next step.
>
> **Rules for Claude Code:**
> - Read `PROJECT_OVERVIEW.md`, `REQUIREMENTS.md`, and `ARCHITECTURE.md` before starting any step.
> - Read `TEST_REFERENCE.md` before writing any test.
> - Complete exactly one step per session. Do not proceed to the next step without explicit approval.
> - `ctest` must exit 0 before declaring any step complete.
> - Every new function and every header must carry a traceability comment.
> - Do not create any class not listed in `ARCHITECTURE.md`.

---

## Step 0 — Baseline Verification  *(Pre-condition, not a build task)*

**What this is:** Confirm the existing codebase is in a known-good state before any new work begins.
This step involves no code changes. If anything fails here, it must be resolved before Step 1.

### Actions
1. Run the full build and test suite:
   ```bash
   cmake -B build -DCMAKE_BUILD_TYPE=Debug
   cmake --build build
   ctest --test-dir build --output-on-failure
   ```
2. Confirm all 5 tests pass:
   - `NoteCreation.HappyPath_TextNoteCreatedAndStored`
   - `NoteCreation.EmptyTitle_ThrowsInvalidArgument`
   - `NoteCreation.TwoNotesWithSameTitle_ReceiveDistinctUUIDs`
   - `SearchByTitle.CaseInsensitive_UpperQueryMatchesLowerTitle`
   - `SearchByTitle.NoMatch_ReturnsEmptyVector`
3. Confirm `CLAUDE.md` hard constraints are satisfied:
   ```bash
   # No raw new/delete
   grep -rn "\bnew \|\bdelete\b" src/ --include="*.cpp" --include="*.h"
   # SecureNote ciphertext type
   grep -n "ciphertext_" src/model/SecureNote.h
   # No std::map for notes_ (must be unordered_map)
   grep -n "std::map" src/controller/NoteManager.h
   ```
4. Confirm `planning/library-decision.md` exists (NFR-4 requires it be committed).
5. Review `docs/dod-checklist.md` — confirm Sprint Zero items were checked off.

### Verification Gate (Jonathan)
- [ ] `cmake --build` exits 0
- [ ] `ctest` exits 0, 5/5 tests pass
- [ ] No raw `new`/`delete` found in src/
- [ ] `ciphertext_` is `std::vector<std::byte>`
- [ ] `notes_` is `std::unordered_map`
- [ ] `CLAUDE.md` is present and current
- [ ] `planning/library-decision.md` exists and is committed

**→ Only proceed to Step 1 after all boxes are checked.**

---

## Step 1 — Add Missing Tests for Existing Behavior (FR-6 / FR-1a)

**What this is:** Two tests cover existing behavior that is implemented but untested. Adding them
closes the PR #1 gap and completes FR-1a test coverage before any new feature work begins.

**Dependency on Step 0:** Step 0 is a pre-condition (baseline must be clean). Note: Step 1 has no
technical dependency on Step 0 beyond the baseline being green — this is a workflow gate to ensure
no new tests are added on top of a broken build.

**Why now:** These are the smallest possible additions. They close existing gaps before new implementation
begins.

### Requirements Satisfied
- FR-6: "An empty query shall return the full collection, not an error."
- FR-1a: "Title exceeding 255 characters shall be rejected."

### Files to Modify
- `test/test_note_creation.cpp` — add tests D4 and A4

### What to Implement

**Test D4:**
```
Test ID: D4
Name: SearchByTitle.EmptyQuery_ReturnsAllNotes
Level: Unit
Requirement: FR-6
```
- Create a NoteManager with 3 TextNotes with distinct titles
- Call `searchByTitle("")`
- Assert the result size equals 3
- Assert no exception is thrown

**Test A4:**
```
Test ID: A4
Name: NoteCreation.RejectsTitleExceeding255Chars
Level: Unit
Requirement: FR-1a
```
- Create a title string of 256 'x' characters
- Call `factory.create("text", longTitle)`
- Assert `std::invalid_argument` is thrown

### Traceability Comments Required
```cpp
// Traceability: FR-6 (refined) | UML: NoteManager.searchByTitle
// Traceability: FR-1a (refined) | UML: NoteFactory.create
```

### Verification Gate (Jonathan)
- [ ] `ctest` exits 0, **7/7 tests pass**
- [ ] D4 test name: `SearchByTitle.EmptyQuery_ReturnsAllNotes`
- [ ] A4 test name: `NoteCreation.RejectsTitleExceeding255Chars`
- [ ] Traceability comments present on both tests
- [ ] No other files were modified

---

## Step 2 — Edit TextNote (FR-2 / US-02)

**What this is:** Implement the ability to update a TextNote's title or body, with atomic semantics,
title validation, and automatic `lastModifiedAt` timestamp update.

**Dependency:** Step 1 must be complete (7/7 tests passing).

### Requirements Satisfied
- FR-2: Edit title or body with atomic update and timestamp refresh
- FR-1a: Same title-length validation rules apply to edits

### Files to Create or Modify
- `src/model/Note.h` — add `lastModifiedAt_` field, `getLastModifiedAt()` accessor, and `Status` enum definition
- `src/model/Note.cpp` — initialize `lastModifiedAt_` in constructor (same value as `createdAt_`)
- `src/controller/NoteManager.h` — add `editTitle(UUID, string)` and `editBody(UUID, string)` declarations
- `src/controller/NoteManager.cpp` — implement both edit methods
- `test/test_note_creation.cpp` — add tests A8, A9, A10

### What to Implement

**Status enum — define once in `src/model/Note.h`:**
```cpp
// Traceability: FR-2, FR-3, FR-5a | UML: Status
enum class Status { OK, NOT_FOUND, INVALID_INPUT };
```

**Note.h additions:**
```cpp
// Traceability: FR-2 (refined) | UML: Note.getLastModifiedAt
std::time_t getLastModifiedAt() const;
```

**NoteManager interface additions:**
```cpp
// Updates the title of an existing TextNote. Returns Status.
// Validates title per FR-1a. Fails atomically on invalid input or missing UUID.
// Traceability: FR-2 (refined) | UML: NoteManager.editTitle
Status editTitle(const UUID& uuid, const std::string& newTitle);

// Updates the body of an existing TextNote. Returns Status.
// Missing UUID returns Status::NOT_FOUND without throwing.
// Traceability: FR-2 (refined) | UML: NoteManager.editBody
Status editBody(const UUID& uuid, const std::string& newBody);
```

**Atomic semantics:** Validate first, then assign. If title validation fails, the existing title
must be unchanged. Neither field partially updates.

**lastModifiedAt update:** On every successful edit (title or body), set `lastModifiedAt_` to
`std::time(nullptr)`.

**TextNote-only constraint:** `editTitle` and `editBody` operate on `TextNote` only. If the UUID
resolves to a `VoiceNote` or `SecureNote`, return `Status::INVALID_INPUT`.

### Tests to Write (from TEST_REFERENCE.md)
- **A8** — `EditTextNote.UpdatesBodyAndTimestamp`: edit body, verify body changed and `lastModifiedAt` updated, `createdAt` unchanged
- **A9** — `EditTextNote.RejectsEmptyTitle`: attempt empty-title edit, verify note unchanged and `Status::INVALID_INPUT` returned
- **A10** — `EditTextNote.MissingUUID_ReturnsNotFound`: edit nonexistent UUID, verify `Status::NOT_FOUND`

### Verification Gate (Jonathan)
- [ ] `ctest` exits 0, **10/10 tests pass**
- [ ] `lastModifiedAt_` field and `getLastModifiedAt()` exist on Note
- [ ] `Status` enum is defined in `Note.h` (not redefined elsewhere)
- [ ] Empty title edit leaves note fully unchanged (verified by A9)
- [ ] Traceability comments on all new methods
- [ ] No raw `new`/`delete` introduced

---

## Step 3 — Delete Note (FR-3 / US-03)

**What this is:** Update `NoteManager::remove()` to return `Status`, add SecureNote ciphertext
zeroing, and achieve complete test coverage of delete behavior.

**Dependency:** Step 2 must be complete (10/10 tests passing). Step 3 depends on `Status` enum
from Step 2.

### Requirements Satisfied
- FR-3: Delete by UUID; release resources; SecureNote zeroing; `Status` return

### Files to Modify
- `src/controller/NoteManager.h` — change `remove()` return type from `void` to `Status`
- `src/controller/NoteManager.cpp` — implement deletion with SecureNote ciphertext zeroing
- `src/model/SecureNote.h` — add `getCiphertextMutable()` returning `std::vector<std::byte>&`
- `src/model/SecureNote.cpp` — implement `getCiphertextMutable()`
- `test/test_note_creation.cpp` — add tests A11, A12, A13, A14

### What to Implement

**Updated NoteManager signature:**
```cpp
// Removes note by UUID. Returns Status::OK if deleted, Status::NOT_FOUND if absent.
// For SecureNote: zeroes ciphertext_ before deallocation.
// Traceability: FR-3 (refined) | UML: NoteManager.remove
Status remove(const UUID& uuid);
```

**SecureNote ciphertext zeroing (FR-3 / SPR-1):**
```cpp
// Before erase, zero ciphertext if SecureNote
if (auto* sn = dynamic_cast<SecureNote*>(it->second.get())) {
    auto& bytes = sn->getCiphertextMutable();
    std::fill(bytes.begin(), bytes.end(), std::byte{0});
}
notes_.erase(it);
```

**SecureNote addition:**
```cpp
// Returns mutable reference to ciphertext for zeroing on delete.
// Traceability: FR-3 (refined) | UML: SecureNote.getCiphertextMutable
std::vector<std::byte>& getCiphertextMutable();
```

### Tests to Write (from TEST_REFERENCE.md)
- **A11** — `DeleteNote.RemovesNoteFromCollection`: add note, delete it, verify `findByUUID` returns nullptr
- **A12** — `DeleteNote.MissingUUID_ReturnsNotFound`: delete nonexistent UUID, verify `Status::NOT_FOUND`
- **A13** — `DeleteNote.OtherNotesUntouched`: add 3 notes, delete 1 by UUID, verify other 2 still retrievable
- **A14** — `DeleteNote.SecureNoteCiphertextZeroed`: create a SecureNote, add data to ciphertext, delete, verify bytes are all zero via getCiphertextMutable() before erase

### Verification Gate (Jonathan)
- [ ] `ctest` exits 0, **14/14 tests pass**
- [ ] `remove()` returns `Status`, not `void`
- [ ] A14 passes: SecureNote ciphertext zeroed before deletion
- [ ] No other notes affected by deletion (A13)
- [ ] Traceability comments on all new/modified methods

---

## Step 4 — FileStorage: Save and Load Round-Trip (FR-4 / US-04)

**What this is:** Implement `FileStorage` — the concrete `StorageInterface` that serializes notes
to `notes.json` and deserializes them on startup. This step covers the happy path only.

**Dependency:** Step 3 must be complete (14/14 tests passing).

### Requirements Satisfied
- FR-4: JSON persistence, NoteFactory reconstruction on load, UUID collision handling
- FR-4a: Missing file is not an error; start empty, create on first save

### Files to Create or Modify
- `src/storage/FileStorage.h` — new file
- `src/storage/FileStorage.cpp` — new file
- `src/controller/NoteFactory.h` — no signature change needed; `reconstructRecord` already declared
- `src/controller/NoteFactory.cpp` — implement `reconstructRecord` (currently throws stub)
- `CMakeLists.txt` — add `src/storage/FileStorage.cpp` to `astra_core` sources
- `test/test_note_creation.cpp` — add tests C1, C2, C3

### What to Implement

**FileStorage class:**
```cpp
class FileStorage final : public StorageInterface {
public:
    explicit FileStorage(std::string filePath);
    void saveNote(const Note& note) override;
    std::vector<std::unique_ptr<Note>> loadNotes() override;
    // Preferred: also implement saveAll for atomic write
    void saveAll(const std::unordered_map<UUID, std::unique_ptr<Note>>& notes);
private:
    std::string filePath_;
};
```

**Resolve the saveAll design question (see REQUIREMENTS.md FR-4):** Implement `saveAll` as an
additional method and have `NoteManager::persistAll()` call it directly (bypassing the per-note
`saveNote` loop). Document this decision inline in `FileStorage.h`.

**Atomic write (FR-4):**
```
1. Serialize all notes to JSON in memory (nlohmann::json array)
2. Write JSON to filePath_ + ".tmp"
3. std::rename((filePath_ + ".tmp").c_str(), filePath_.c_str())
```

**JSON schema per note:**
```json
{
  "uuid": "...",
  "type": "text",
  "title": "...",
  "body": "...",
  "createdAt": 1234567890,
  "lastModifiedAt": 1234567890
}
```
SecureNote: `"ciphertext"` field is a hex-encoded string of bytes. No `"body"` field.

**NoteFactory.reconstructRecord:** Parse the `"type"` field, call the appropriate constructor
with the persisted UUID, title, and timestamps. Do not generate a new UUID — use the stored one.

**FR-4a:** If `filePath_` does not exist, `loadNotes()` returns an empty vector. No error.

### Tests to Write (from TEST_REFERENCE.md)

Use a GTest fixture with a per-test temporary directory (SetUp/TearDown).

- **C1** — `Persistence.RoundTrip_NotePreservesAllFields`
- **C2** — `Persistence.LoadsAllThreeConcreteTypes`
- **C3** — `Persistence.MissingFile_ReturnsEmptyCollection`

### Verification Gate (Jonathan)
- [ ] `ctest` exits 0, **17/17 tests pass**
- [ ] `FileStorage.h` and `.cpp` exist
- [ ] `NoteFactory::reconstructRecord` is implemented (no longer throws)
- [ ] Atomic write via tmp-then-rename confirmed (`grep -n ".tmp" src/storage/FileStorage.cpp`)
- [ ] Missing file returns empty vector (C3)
- [ ] Traceability comments on all new methods

---

## Step 5 — FileStorage: Edge Cases and Quarantine (FR-4b)

**What this is:** Extend FileStorage to handle corrupt files and partially corrupt files.

**Dependency:** Step 4 must be complete (17/17 tests passing).

### Requirements Satisfied
- FR-4b: Quarantine corrupt file; per-record skip for partially corrupt files
- FR-4: UUID collision on load → discard second, log conflict

### Files to Modify
- `src/storage/FileStorage.cpp` — add quarantine logic and per-record error handling
- `test/test_note_creation.cpp` — add tests C4, C5, C6

### What to Implement

**Quarantine logic (FR-4b):**
```
1. Try to parse the file as JSON
2. If parse fails entirely:
   a. Rename to filePath_ + ".quarantine." + std::to_string(std::time(nullptr))
   b. Log to std::cerr
   c. Return empty vector
3. If parse succeeds but individual record is malformed:
   a. Log to std::cerr and skip that record
   b. Continue loading the rest — do NOT stop at first bad record
4. If a loaded UUID already exists in the output:
   a. Log to std::cerr: "duplicate UUID discarded"
   b. Discard second, keep first
```

### Tests to Write (from TEST_REFERENCE.md)
- **C4** — `Persistence.CorruptFile_QuarantinesAndReturnsEmpty`
- **C5** — `Persistence.PartiallyCorruptFile_LoadsValidRecords`
- **C6** — `Persistence.DuplicateUUID_DiscardsSecond`

### Verification Gate (Jonathan)
- [ ] `ctest` exits 0, **20/20 tests pass**
- [ ] Quarantine file created with Unix timestamp in name (C4)
- [ ] Partial corruption loads all valid records (C5) — not just records before the bad one
- [ ] UUID collision: first record wins, second discarded (C6)
- [ ] All error paths log to `std::cerr`

---

## Step 6 — SecureNote Passphrase Gate (FR-5 / US-05)

**What this is:** Implement `SecureNote::lock()` and `SecureNote::unlock()` using the injected
`EncryptionEngine`. Unit tests use a `MockEncryptionEngine`. No real crypto linked.

**Dependency:** Step 5 must be complete (20/20 tests passing). Note: Step 6 does not technically
depend on FileStorage edge cases; it is sequenced here as a workflow decision to complete persistence
before adding crypto behavior.

### Requirements Satisfied
- FR-5: Passphrase gate; plaintext not retained; decryption buffer zeroed
- FR-5a: Typed error codes (`SecureNoteError`)
- SPR-1: No plaintext in any member, log, or file
- SPR-2: Tests use `MockEncryptionEngine`, not real crypto

### Files to Modify
- `src/model/SecureNote.h` — add `lock()`, `unlock()` declarations; add `SecureNoteError` enum
- `src/model/SecureNote.cpp` — implement both methods
- `test/test_note_creation.cpp` — add tests B1, B2, B3, B4, B5; define `MockEncryptionEngine` inline

### What to Implement

**SecureNoteError enum — define in `src/model/SecureNote.h`:**
```cpp
// Traceability: FR-5a (refined) | UML: SecureNoteError
enum class SecureNoteError { WRONG_PASSPHRASE, INVALID_INPUT };
```

**SecureNote interface additions:**
```cpp
// Traceability: FR-5 (refined) | UML: SecureNote.lock
Status lock(const std::string& plaintext, const std::string& passphrase);

// Traceability: FR-5 (refined) | UML: SecureNote.unlock
std::variant<std::string, SecureNoteError> unlock(const std::string& passphrase);
```

**Passphrase validation (FR-5a):**
- Empty passphrase → `SecureNoteError::INVALID_INPUT` (do not call `engine_.decrypt`)
- Passphrase `.size() < 8` → `SecureNoteError::INVALID_INPUT`
- Use `.size()`, not `strlen()`

**Plaintext zeroing (FR-5 / SPR-1):**
```cpp
// In lock(): zero local plaintext copy after encryption
std::fill(localPlaintext.begin(), localPlaintext.end(), '\0');

// In unlock(): zero decryption buffer whether success or failure
std::fill(decryptBuf.begin(), decryptBuf.end(), '\0');
```

**MockEncryptionEngine (inline in test file):**
```cpp
class MockEncryptionEngine : public EncryptionEngine {
public:
    std::vector<std::byte> encrypt(const std::string&, const std::string&) override {
        return {std::byte{0xDE}, std::byte{0xAD}};
    }
    std::string decrypt(const std::vector<std::byte>&, const std::string& pass) override {
        if (pass == correctPass_) return plaintext_;
        throw std::runtime_error("wrong passphrase");
    }
    std::string correctPass_ = "testpass1";
    std::string plaintext_   = "secret content";
};
```

### Tests to Write (from TEST_REFERENCE.md)
- **B1** — `SecureNote.CorrectPassphrase_ReturnsPlaintext`
- **B2** — `SecureNote.WrongPassphrase_ReturnsError`
- **B3** — `SecureNote.EmptyPassphrase_RejectedBeforeDecrypt`
- **B4** — `SecureNote.ShortPassphrase_ReturnsInvalidInput`
- **B5** — `SecureNote.AfterUnlock_NoPlaintextInPublicAPI`

### Verification Gate (Jonathan)
- [ ] `ctest` exits 0, **25/25 tests pass**
- [ ] `unlock()` returns `std::variant`, not a raw string
- [ ] `SecureNoteError` enum is defined in `SecureNote.h`
- [ ] No crypto library linked (mock only — `grep -n "OpenSSL\|AES" src/model/SecureNote.cpp` returns nothing)
- [ ] grep confirms no plaintext member: `grep -n "plaintext" src/model/SecureNote.h`

---

## Step 7 — AESEngine: Real Encryption (SPR-1)

**What this is:** Implement `AESEngine` — the concrete AES-256-GCM implementation via OpenSSL
libcrypto. Wire it into CMake. Integration test B6 verifies FileStorage serializes ciphertext only.

**Dependency:** Step 6 must be complete (25/25 tests passing).

**Pre-condition:** `planning/library-decision.md` must include OpenSSL justification (NFR-4) before
this step begins.

### Requirements Satisfied
- SPR-1: SecureNote JSON records contain only ciphertext — no plaintext
- SPR-2: AESEngine is the concrete implementation; interface unchanged

### Files to Create or Modify
- `src/encryption/AESEngine.h` — new file
- `src/encryption/AESEngine.cpp` — new file
- `CMakeLists.txt` — link OpenSSL to `astra_core`
- `test/test_note_creation.cpp` — add test B6

### What to Implement

**AESEngine class:**
```cpp
class AESEngine final : public EncryptionEngine {
public:
    // AES-256-GCM. Key derived from passphrase via PBKDF2-SHA256.
    // Returns IV || tag || ciphertext as a single vector<byte>.
    // Traceability: SPR-1, SPR-2 | UML: AESEngine.encrypt
    std::vector<std::byte> encrypt(const std::string& plaintext,
                                    const std::string& passphrase) override;

    // Extracts IV and tag, decrypts, verifies authentication tag.
    // Throws on authentication failure.
    // Traceability: SPR-1, SPR-2 | UML: AESEngine.decrypt
    std::string decrypt(const std::vector<std::byte>& ciphertext,
                        const std::string& passphrase) override;
};
```

**CMake change (note correct variable name):**
```cmake
if(OPENSSL_FOUND)
    target_link_libraries(astra_core PUBLIC OpenSSL::Crypto)
endif()
```

### Test to Write (from TEST_REFERENCE.md)
- **B6** — `SecureNote.FileStorage_CiphertextOnly` (Integration):
  Use real AESEngine. Save a SecureNote via FileStorage. Load the raw JSON file.
  Parse it. Assert no `"body"` or `"plaintext"` field exists. Assert `"ciphertext"` field exists.

### Verification Gate (Jonathan)
- [ ] `ctest` exits 0, **26/26 tests pass**
- [ ] `cmake --build` succeeds with OpenSSL linked (no linker errors)
- [ ] B6 integration test passes
- [ ] `grep -n "plaintext\|body" <output notes.json>` shows no plaintext field for SecureNote records
- [ ] `AESEngine` is only referenced through `EncryptionEngine&` (grep for `AESEngine` in NoteManager, SecureNote, FileStorage headers — must be zero)

---

## Step 8 — Wire All Features into CLI (US-01 through US-06)

**What this is:** Update `main.cpp` to expose edit, delete, persistence, and SecureNote passphrase
gate through the CLI menu. Replace `NullStorage` with `FileStorage`.

**Dependency:** Step 7 must be complete (26/26 tests passing).

### Requirements Satisfied
- FR-2: Edit TextNote via CLI
- FR-3: Delete note via CLI
- FR-4, FR-4a, FR-4b: Load on startup, save on exit
- FR-5: SecureNote passphrase gate via CLI

### Files to Modify
- `src/main.cpp` — extend menu commands; replace `NullStorage` with `FileStorage`; add `AESEngine`

### Important: NullStorage in tests is NOT removed
`NullStorage` is defined inline in `test/test_note_creation.cpp` as a separate inline struct.
This is unaffected by this step. Only the `NullStorage` in `src/main.cpp` is replaced.

### What to Implement

**Updated menu:**
```
[n] new note      — prompt for type (text/voice/secure) and title
[l] list notes    — renderNoteList()
[v] view note     — prompt for UUID, renderNote()
[e] edit note     — prompt for UUID, field (title/body), new value
[d] delete note   — prompt for UUID, confirm, remove()
[s] settings      — (stub — "Settings: not yet implemented.")
[q] quit          — persistAll() then exit
```

**FileStorage and AESEngine wiring:**
```cpp
AESEngine engine;
FileStorage storage("~/.astranotes/notes.json");
NoteManager manager(factory, storage);
manager.loadAll();   // on startup
// ...
manager.persistAll(); // on quit
```

### Verification Gate (Jonathan)
- [ ] `ctest` exits 0, **26/26 tests still pass** (no regression)
- [ ] [manual] Application starts and loads existing notes from `~/.astranotes/notes.json`
- [ ] [manual] New notes persist across quit-and-relaunch
- [ ] [manual] Edit and delete work end-to-end via CLI
- [ ] [manual] SecureNote creation and unlocking work via CLI
- [ ] `NullStorage` removed from `src/main.cpp` only (confirmed by grep; test file unaffected)

---

## Step 9 — VersionHistory Population (NFR-3)

**What this is:** Implement `VersionHistory::addEntry()` and call it from NoteManager's edit path
so every body edit creates a VersionEntry snapshot.

**Dependency:** Step 8 must be complete and verified.

### Requirements Satisfied
- NFR-3: VersionHistory tracks edit history per note (read-only; no persistence)

### Files to Modify
- `src/model/Note.h` — add non-const `getVersionHistory()` overload
- `src/model/Note.cpp` — implement non-const overload
- `src/model/VersionHistory.h` — add `addEntry(VersionEntry)` declaration
- `src/model/VersionHistory.cpp` — implement `addEntry`
- `src/controller/NoteManager.cpp` — call `addEntry` after every successful `editBody`
- `test/test_note_creation.cpp` — add test E1

### What to Implement

**VersionHistory addition:**
```cpp
// Appends a snapshot entry after each body edit.
// Traceability: NFR-3 | UML: VersionHistory.addEntry
void addEntry(VersionEntry entry);
```

**Note non-const overload:**
```cpp
// Non-const version allows NoteManager to append entries after edits.
// Traceability: NFR-3 | UML: Note.getVersionHistory
VersionHistory& getVersionHistory();
```

**NoteManager.editBody update:** After updating `body_` and `lastModifiedAt_`, append:
```cpp
VersionEntry entry{newBody, std::time(nullptr)};
note->getVersionHistory().addEntry(entry);
```

### Test to Write
- **E1** — `VersionHistory.EditBodyAppendsEntry`:
  Edit a TextNote body twice. Assert `getVersionHistory().getEntries().size() == 2`.
  Assert first entry has correct snapshotBody and timestamp. Assert second entry differs.

  ```
  // Traceability: NFR-3 | UML: VersionHistory.addEntry
  TEST(VersionHistory, EditBodyAppendsEntry) { ... }
  ```

### Verification Gate (Jonathan)
- [ ] `ctest` exits 0, all tests pass
- [ ] `VersionHistory::getEntries()` returns 2 entries after 2 body edits (E1)
- [ ] Title edits do NOT append VersionHistory entries (body-only behavior)
- [ ] VersionHistory entries are not persisted to JSON (FileStorage ignores history_)

---

## Step 10 — Final Audit and DoD Checklist

**What this is:** A full project review against the Definition of Done. No new features.

**Dependency:** All prior steps complete.

### Actions
1. Run full test suite and confirm all tests pass.
2. Run grep checks:
   ```bash
   grep -rn "\bnew \|\bdelete\b" src/         # must be zero results
   grep -rn "std::map" src/controller/         # must be zero (unordered_map only)
   grep -rn "plaintext" src/model/SecureNote.h # must be zero member variables
   grep -rn "OPENSSL\|AES" src/model/ src/controller/ src/storage/ src/view/ --include="*.h"
   # above: must be zero (AESEngine only accessible via EncryptionEngine& references)
   ```
3. Verify every header has `#pragma once` and a traceability comment.
4. Verify every implemented function has a traceability comment.
5. Verify `planning/library-decision.md` covers both nlohmann/json AND OpenSSL (NFR-4).
6. Add SPR-3 note to `README.md`: "Concurrent access to the same storage file by two application instances is undefined behavior in v1.0."
7. Tick all boxes in `docs/dod-checklist.md`.
8. Update `PROJECT_OVERVIEW.md` implementation state table to reflect final state.

### Final Verification Gate (Jonathan)
- [ ] All tests pass
- [ ] All 4 grep checks return zero results
- [ ] Every header has `#pragma once` and a traceability comment
- [ ] `planning/library-decision.md` covers nlohmann/json and OpenSSL
- [ ] `README.md` documents SPR-3 concurrent access limitation
- [ ] `docs/dod-checklist.md` fully checked off
- [ ] `PROJECT_OVERVIEW.md` implementation state table is current
