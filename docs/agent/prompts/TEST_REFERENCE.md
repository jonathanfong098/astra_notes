# AstraNotes — Test Reference

<!-- CHANGELOG
- Corrected A1 level from Unit to Integration (tests NoteFactory + NoteManager cooperation)
- Corrected A6 annotation: A6 behavior is bundled inside A1; labeled as "(bundled in A1)"
- Corrected A7 annotation: "covered by A1" was incorrect — A1 tests happy path only; A7 needs dedicated test or explicit coverage note
- Added A14: DeleteNote.SecureNoteCiphertextZeroed (FR-3, SPR-1) — added by BUILD_PLAN Step 3 evaluation
- Added E1: VersionHistory.EditBodyAppendsEntry (NFR-3) — added by BUILD_PLAN Step 9 evaluation
- Changed A4 from "optionally add" to assigned Step 1 (alongside D4)
- Updated all cumulative test counts to definite numbers (A4 no longer optional)
- Added coverage matrix with all requirement IDs
- Fixed D1 coverage note: D3 covers case-insensitivity; D1's "all matches, no others" needs explicit verification
- Added note that D5 uses FailOnCallStorage to prove absence of I/O
-->

> **Purpose of this document:** The authoritative test catalog. Claude Code reads this before writing
> any test. Every test has an ID, level, description, requirement mapping, and current status.
> Update the Status column whenever a test is implemented or a step completes.
>
> **Test levels:**
> - **Unit** — tests a single class in isolation; mocks all collaborators; no real filesystem or crypto
> - **Integration** — two or more real classes cooperate; filesystem access permitted in a temp directory

---

## Status Legend

| Symbol | Meaning |
|---|---|
| ✅ | Implemented and passing |
| 🔲 | Outlined (specified here, not yet written) |
| ❌ | Not yet specified or out of scope |

---

## Feature A — Note Creation (US-01 / FR-1)

| Test ID | Level | Test Name | What It Verifies | Requirement | Status |
|---|---|---|---|---|---|
| A1 | Integration | `NoteCreation.HappyPath_TextNoteCreatedAndStored` | Valid title/type → TextNote with UUID and correct type; add to NoteManager; findByUUID retrieves it | US-01 AC-1, FR-1, NFR-1 | ✅ |
| A2 | Unit | `NoteCreation.EmptyTitle_ThrowsInvalidArgument` | Empty and whitespace-only titles → `std::invalid_argument`; collection unchanged | US-01 AC-2, FR-1a | ✅ |
| A3 | Unit | `NoteCreation.TwoNotesWithSameTitle_ReceiveDistinctUUIDs` | Two notes with identical titles → distinct UUIDs | US-01 AC-3, FR-1 | ✅ |
| A4 | Unit | `NoteCreation.RejectsTitleExceeding255Chars` | Title of 256 chars → `std::invalid_argument` | FR-1a | 🔲 |
| A5 | Unit | *(covered by A2)* | Whitespace-only (tab, newline) → `std::invalid_argument` | FR-1a | ✅ |
| A6 | Integration | *(bundled in A1)* | add() transfers ownership; findByUUID returns note by key | NFR-1 | ✅ |
| A7 | Unit | `NoteManager.FindByUUIDReturnsNullForMissingUUID` | findByUUID("nonexistent") → nullptr; no exception | NFR-2 | 🔲 |

> **Note on A7:** Originally annotated as "covered by A1" — this is incorrect. A1 tests finding
> an existing note, not a missing one. A7 needs a dedicated test. Add in Step 1 alongside D4 and A4,
> or confirm coverage exists elsewhere before marking ✅.

---

## Feature A (continued) — Edit TextNote (US-02 / FR-2)

| Test ID | Level | Test Name | What It Verifies | Requirement | Status |
|---|---|---|---|---|---|
| A8 | Unit | `EditTextNote.UpdatesBodyAndTimestamp` | Edit body → body changed; `lastModifiedAt` updated; `createdAt` unchanged | US-02 AC-1, FR-2 | 🔲 |
| A9 | Unit | `EditTextNote.RejectsEmptyTitle` | Empty title edit → note fully unchanged; `Status::INVALID_INPUT` returned | US-02 AC-2, FR-2, FR-1a | 🔲 |
| A10 | Unit | `EditTextNote.MissingUUID_ReturnsNotFound` | Edit on nonexistent UUID → `Status::NOT_FOUND`; no exception | US-02 AC-3, FR-2 | 🔲 |

---

## Feature A (continued) — Delete Note (US-03 / FR-3)

| Test ID | Level | Test Name | What It Verifies | Requirement | Status |
|---|---|---|---|---|---|
| A11 | Unit | `DeleteNote.RemovesNoteFromCollection` | Add then delete → findByUUID returns nullptr | US-03 AC-1, FR-3 | 🔲 |
| A12 | Unit | `DeleteNote.MissingUUID_ReturnsNotFound` | Delete nonexistent UUID → `Status::NOT_FOUND`; no exception | US-03 AC-2, FR-3, NFR-2 | 🔲 |
| A13 | Unit | `DeleteNote.OtherNotesUntouched` | Add 3 notes, delete 1 → other 2 still retrievable by UUID | US-03 AC-4, FR-3 | 🔲 |
| A14 | Unit | `DeleteNote.SecureNoteCiphertextZeroed` | Create SecureNote with ciphertext, delete it, verify all bytes are zero (via getCiphertextMutable before erase) | US-03 AC-3, FR-3, SPR-1 | 🔲 |

---

## Feature B — SecureNote Passphrase Gate (US-05 / FR-5)

All B-series unit tests use `MockEncryptionEngine` — no real crypto library is linked.

```cpp
// Inline in test file — do NOT create a separate class file
class MockEncryptionEngine : public EncryptionEngine {
public:
    std::vector<std::byte> encrypt(const std::string&, const std::string&) override {
        return {std::byte{0xDE}, std::byte{0xAD}};  // canned ciphertext
    }
    std::string decrypt(const std::vector<std::byte>&, const std::string& pass) override {
        if (pass == correctPass_) return plaintext_;
        throw std::runtime_error("wrong passphrase");
    }
    std::string correctPass_ = "testpass1";
    std::string plaintext_   = "secret content";
};
```

| Test ID | Level | Test Name | What It Verifies | Requirement | Status |
|---|---|---|---|---|---|
| B1 | Unit | `SecureNote.CorrectPassphrase_ReturnsPlaintext` | Correct passphrase → mock.decrypt called; plaintext returned to caller | US-05 AC-1, FR-5 | 🔲 |
| B2 | Unit | `SecureNote.WrongPassphrase_ReturnsError` | Wrong passphrase → `WRONG_PASSPHRASE` error code; no plaintext exposed | US-05 AC-2, FR-5a | 🔲 |
| B3 | Unit | `SecureNote.EmptyPassphrase_RejectedBeforeDecrypt` | Empty passphrase → `INVALID_INPUT`; mock.decrypt never called | US-05 AC-3, FR-5a | 🔲 |
| B4 | Unit | `SecureNote.ShortPassphrase_ReturnsInvalidInput` | Passphrase < 8 chars → `INVALID_INPUT`; distinct from `WRONG_PASSPHRASE` | FR-5, FR-5a | 🔲 |
| B5 | Unit | `SecureNote.AfterUnlock_NoPlaintextInPublicAPI` | After unlock completes, no plaintext reachable through any public accessor | US-05 AC-1, SPR-1 | 🔲 |
| B6 | Integration | `SecureNote.FileStorage_CiphertextOnly` | FileStorage saves SecureNote; JSON has `"ciphertext"` field, no `"body"` or `"plaintext"` | US-05 AC-4, SPR-1 | 🔲 |

> **B6 requires real AESEngine** — do not write B6 until Step 7 (AESEngine) is complete.
> B1–B5 use MockEncryptionEngine and can be written in Step 6.

---

## Feature C — JSON Persistence (US-04 / FR-4)

All C-series tests use a **per-test temporary directory** created by a GTest fixture. Never write to a real user path.

```cpp
class FileStorageTest : public ::testing::Test {
protected:
    std::string tmpPath_;
    void SetUp() override {
        // Create a unique temp file path for this test
        tmpPath_ = std::filesystem::temp_directory_path() / "astranotes_test.json";
    }
    void TearDown() override {
        std::filesystem::remove(tmpPath_);
        std::filesystem::remove(tmpPath_ + ".quarantine");  // clean up quarantine files
    }
};
```

| Test ID | Level | Test Name | What It Verifies | Requirement | Status |
|---|---|---|---|---|---|
| C1 | Integration | `Persistence.RoundTrip_NotePreservesAllFields` | Save TextNote, reload; UUID/title/type/createdAt/lastModifiedAt all match exactly | US-04 AC-1, FR-4 | 🔲 |
| C2 | Integration | `Persistence.LoadsAllThreeConcreteTypes` | File with TextNote + VoiceNote + SecureNote; each loads as correct concrete type via NoteFactory | US-04 AC-2, FR-4 | 🔲 |
| C3 | Integration | `Persistence.MissingFile_ReturnsEmptyCollection` | FileStorage at nonexistent path; loadNotes() returns empty vector; no exception | FR-4a | 🔲 |
| C4 | Integration | `Persistence.CorruptFile_QuarantinesAndReturnsEmpty` | Write garbage to file; loadNotes() returns empty; quarantine file exists with Unix timestamp in name | US-04 AC-3, FR-4b | 🔲 |
| C5 | Integration | `Persistence.PartiallyCorruptFile_LoadsValidRecords` | File has 3 valid + 1 malformed record; loadNotes() returns exactly 3 notes | FR-4b | 🔲 |
| C6 | Integration | `Persistence.DuplicateUUID_DiscardsSecond` | File has 2 records with same UUID; loadNotes() returns 1 note (first wins) | FR-4 | 🔲 |

---

## Feature D — Title Substring Search (US-06 / FR-6)

All D-series tests use `FailOnCallStorage` to prove no I/O occurs during search (FR-6 / US-06 AC-3).

```cpp
// Inline in test file — fails the test if save/load/remove is called during search
class FailOnCallStorage : public StorageInterface {
public:
    void saveNote(const Note&) override { FAIL() << "Search must not call saveNote"; }
    std::vector<std::unique_ptr<Note>> loadNotes() override {
        FAIL() << "Search must not call loadNotes";
        return {};
    }
};
```

| Test ID | Level | Test Name | What It Verifies | Requirement | Status |
|---|---|---|---|---|---|
| D1 | Unit | `SearchByTitle.SubstringMatch_ReturnsMatchingOnly` | Substring query → all matching notes returned; non-matching notes excluded | US-06 AC-1, FR-6 | 🔲 |
| D2 | Unit | `SearchByTitle.NoMatch_ReturnsEmptyVector` | Non-matching query → empty vector; no exception | US-06 AC-2, FR-6 | ✅ |
| D3 | Unit | `SearchByTitle.CaseInsensitive_UpperQueryMatchesLowerTitle` | "ALPHA" matches "alpha notes" | FR-6 | ✅ |
| D4 | Unit | `SearchByTitle.EmptyQuery_ReturnsAllNotes` | Empty string query → every note in collection returned | FR-6 | 🔲 |
| D5 | Unit | `SearchByTitle.NoFileIODuringSearch` | searchByTitle() never invokes save/load on FailOnCallStorage | US-06 AC-3, FR-6 | 🔲 |

> **Note on D1:** D3 tests case-insensitivity but does not explicitly verify that *only* matching
> notes are returned (no false positives). D1 should populate a collection with both matching and
> non-matching notes and assert the result contains exactly the matching ones.

---

## Feature E — Version History (NFR-3)

| Test ID | Level | Test Name | What It Verifies | Requirement | Status |
|---|---|---|---|---|---|
| E1 | Unit | `VersionHistory.EditBodyAppendsEntry` | Edit a TextNote body twice; `getVersionHistory().getEntries().size() == 2`; entries have correct body snapshots and timestamps | NFR-3 | 🔲 |

---

## Requirement Coverage Matrix

| Req ID | Tests | Coverage Status |
|---|---|---|
| FR-1 | A1, A2, A3, A4, A6 | ✅ (A4 outlined) |
| FR-1a | A2, A3, A4, A9 | ✅ (A4, A9 outlined) |
| FR-2 | A8, A9, A10 | 🔲 outlined |
| FR-3 | A11, A12, A13, A14 | 🔲 outlined |
| FR-4 | C1, C2, C6 | 🔲 outlined |
| FR-4a | C3 | 🔲 outlined |
| FR-4b | C4, C5 | 🔲 outlined |
| FR-5 | B1, B4 | 🔲 outlined |
| FR-5a | B2, B3, B4 | 🔲 outlined |
| FR-6 | D1, D2, D3, D4, D5 | ⚠️ D4 missing; D1 needs own test |
| NFR-1 | A1, A6 | ✅ |
| NFR-2 | A7, A12 | ⚠️ A7 needs dedicated test |
| NFR-3 | E1 | 🔲 outlined |
| NFR-4 | — | ✅ Process requirement; no test |
| SPR-1 | A14, B5, B6 | 🔲 outlined |
| SPR-2 | B1–B5 (mock) | 🔲 outlined |
| SPR-3 | — | ✅ README requirement; no test |

---

## Test Count Summary

| Step | Tests Added | IDs | Cumulative Total |
|---|---|---|---|
| Step 0 (baseline) | 0 (5 existing) | A1, A2, A3, D2, D3 | **5** |
| Step 1 | 3 | D4, A4, A7 | **8** |
| Step 2 | 3 | A8, A9, A10 | **11** |
| Step 3 | 4 | A11, A12, A13, A14 | **15** |
| Step 4 | 3 | C1, C2, C3 | **18** |
| Step 5 | 3 | C4, C5, C6 | **21** |
| Step 6 | 5 | B1, B2, B3, B4, B5 | **26** |
| Step 7 | 1 | B6 | **27** |
| Step 8 | 0 | — | **27** |
| Step 9 | 2 | E1, D1, D5* | **29** |

> *D1 and D5 are outlined but unassigned to a specific step. Recommend adding D1 and D5 to Step 1
> since both test existing NoteManager behavior with no new implementation required.

---

## Naming Conventions

All test names follow `Feature.Condition_ExpectedOutcome` format:
- `NoteCreation.EmptyTitle_ThrowsInvalidArgument` ✅
- `SearchByTitle.EmptyQuery_ReturnsAllNotes` ✅
- `Persistence.CorruptFile_QuarantinesAndReturnsEmpty` ✅

---

## Testing Constraints

1. **No unit test links a real crypto library.** Use `MockEncryptionEngine` for B1–B5.
2. **No unit test writes to the real filesystem.** Use `NullStorage` or `FailOnCallStorage`.
3. **Integration tests use per-test temp directories.** Fixture must clean up in TearDown.
4. **Search tests must prove no I/O occurs.** Use `FailOnCallStorage` for D1, D4, D5.
5. **Every test must have a traceability comment** citing the requirement ID it validates:
   `// Traceability: FR-6 (refined) | UML: NoteManager.searchByTitle`
