# AstraNotes — Testing Strategy

## Overview

AstraNotes uses a two-level testing strategy enforced through Google Test (GTest) v1.14.0.
All tests live in `test/test_note_creation.cpp` and are discovered automatically by CTest.

**Total tests: 33**
**All tests passing: verified by `ctest --test-dir build --output-on-failure` (see `docs/test-results.txt`)**

---

## Test Levels

### Unit Tests
- Test a single class in complete isolation
- Mock all collaborators using inline stub classes defined at the top of the test file
- No real filesystem access — use `NullStorage` (no-op) or `FailOnCallStorage` (fails the test on any I/O call)
- No real crypto — use `MockEncryptionEngine` (returns canned ciphertext/plaintext)
- Run pre-commit on every change

### Integration Tests
- Two or more real classes cooperate in the test
- Filesystem access is permitted in a per-test temporary directory
- `FileStorageTest` GTest fixture handles setup and teardown; temp files are removed after each test
- Run pre-merge against the feature branch and pre-sprint-review

---

## Injection Boundaries

The architecture is designed so that all external dependencies can be replaced with
test doubles. This satisfies SPR-2 (injectable interfaces) and makes every module
independently testable.

| Production class | Test double | Used in |
|---|---|---|
| `FileStorage` | `NullStorage` (inline) | Unit tests — no I/O needed |
| `FileStorage` | `FailOnCallStorage` (inline) | Search tests — proves zero I/O occurs during search |
| `FileStorage` | Real `FileStorage` + temp dir | Integration tests — real JSON round-trip |
| `AESEngine` | `MockEncryptionEngine` (inline) | SecureNote unit tests — no real crypto linked |
| `AESEngine` | Real `AESEngine` | Integration test B6 — proves ciphertext-only on disk |

---

## Test Suites

| Suite | Test Name | Level | Req ID |
|---|---|---|---|
| NoteCreation | `HappyPath_TextNoteCreatedAndStored` | Integration | FR-1, NFR-1 |
| NoteCreation | `EmptyTitle_ThrowsInvalidArgument` | Unit | FR-1a |
| NoteCreation | `TwoNotesWithSameTitle_ReceiveDistinctUUIDs` | Unit | FR-1 |
| NoteCreation | `AddDuplicateUUID_Throws` | Unit | FR-1 |
| NoteCreation | `RejectsTitleExceeding255Chars` | Unit | FR-1a |
| SearchByTitle | `SubstringMatch_ReturnsMatchingOnly` | Unit | FR-6 |
| SearchByTitle | `CaseInsensitive_UpperQueryMatchesLowerTitle` | Unit | FR-6 |
| SearchByTitle | `NoMatch_ReturnsEmptyVector` | Unit | FR-6 |
| SearchByTitle | `EmptyQuery_ReturnsAllNotes` | Unit | FR-6 |
| SearchByTitle | `NoFileIODuringSearch` | Unit | FR-6 |
| NoteManager | `FindByUUIDReturnsNullForMissingUUID` | Unit | NFR-2 |
| EditTextNote | `UpdatesBodyAndTimestamp` | Unit | FR-2 |
| EditTextNote | `RejectsEmptyTitle` | Unit | FR-2, FR-1a |
| EditTextNote | `MissingUUID_ReturnsNotFound` | Unit | FR-2 |
| EditTextNote | `EditVoiceNote_TitleAndAudioPathEditable` | Unit | FR-2 |
| DeleteNote | `RemovesNoteFromCollection` | Unit | FR-3 |
| DeleteNote | `MissingUUID_ReturnsNotFound` | Unit | FR-3, NFR-2 |
| DeleteNote | `OtherNotesUntouched` | Unit | FR-3 |
| DeleteNote | `SecureNoteCiphertextZeroed` | Unit | FR-3, SPR-1 |
| DeleteNote | `SecureNoteCiphertextMutableZeroingWorks` | Unit | FR-3, SPR-1 |
| FileStorageTest | `RoundTrip_NotePreservesAllFields` | Integration | FR-4 |
| FileStorageTest | `LoadsAllThreeConcreteTypes` | Integration | FR-4 |
| FileStorageTest | `MissingFile_ReturnsEmptyCollection` | Integration | FR-4a |
| FileStorageTest | `CorruptFile_QuarantinesAndReturnsEmpty` | Integration | FR-4b |
| FileStorageTest | `PartiallyCorruptFile_LoadsValidRecords` | Integration | FR-4b |
| FileStorageTest | `DuplicateUUID_DiscardsSecond` | Integration | FR-4 |
| FileStorageTest | `FileStorage_CiphertextOnly` | Integration | SPR-1 |
| VersionHistory | `EditBodyAppendsEntry` | Unit | NFR-3 |
| SecureNote | `CorrectPassphrase_ReturnsPlaintext` | Unit | FR-5 |
| SecureNote | `WrongPassphrase_ReturnsError` | Unit | FR-5a |
| SecureNote | `EmptyPassphrase_RejectedBeforeDecrypt` | Unit | FR-5a |
| SecureNote | `ShortPassphrase_ReturnsInvalidInput` | Unit | FR-5, FR-5a |
| SecureNote | `AfterUnlock_NoPlaintextInPublicAPI` | Unit | FR-5, SPR-1 |

---

## Security Test Constraints

Per SPR-2, no unit test may link a real crypto library. The `MockEncryptionEngine` is
defined inline in the test file (not as a separate class file) and is the only
EncryptionEngine implementation used in B1–B5 unit tests.

Per FR-6, search tests use `FailOnCallStorage` — any invocation of `saveNote()` or
`loadNotes()` during a search immediately fails the test via `FAIL()`, proving the
no-I/O constraint at runtime rather than by inspection.

---

## Running the Tests

```bash
# Full suite
ctest --test-dir build --output-on-failure

# Verbose output with individual test names
ctest --test-dir build --output-on-failure --verbose

# Run a specific suite by name pattern
ctest --test-dir build --output-on-failure -R SearchByTitle
ctest --test-dir build --output-on-failure -R FileStorageTest
ctest --test-dir build --output-on-failure -R SecureNote
```
