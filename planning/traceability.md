# AstraNotes — Requirement Traceability

> **Purpose:** Maps every requirement from the refined baseline (Lab 3.1) to its UML element,
> its implementation in the codebase, and its test coverage.
> This is Sprint Zero deliverable S0-7.

---

## Traceability Table

| Req ID | Requirement (summary) | UML Element | Implementation | Test IDs |
|---|---|---|---|---|
| FR-1 | Create TextNote/VoiceNote/SecureNote with RFC 4122 v4 UUID and title | `NoteFactory.create`, `NoteManager.add` | `NoteFactory.cpp::create()`, `NoteManager.cpp::add()` | A1, A2, A3, A4 |
| FR-1a | Reject empty/whitespace/>255-char titles with descriptive error | `NoteFactory.create` | `NoteFactory.cpp::create()` — trim + size check before UUID generation | A2, A4 |
| FR-2 | Edit TextNote/VoiceNote title or body atomically; update lastModifiedAt | `NoteManager.editTitle`, `NoteManager.editBody` | `NoteManager.cpp::editTitle()`, `::editBody()` — validate first, then assign | A8, A9, A10, A15 |
| FR-3 | Delete note by UUID; zero SecureNote ciphertext before deallocation | `NoteManager.remove` | `NoteManager.cpp::remove()` — SecureNote dynamic_cast + std::fill before erase | A11, A12, A13, A14 |
| FR-4 | Persist all notes to JSON; atomic tmp-then-rename write; UUID collision discard | `FileStorage.saveAll`, `FileStorage.loadNotes`, `NoteFactory.reconstructRecord` | `FileStorage.cpp::saveAll()`, `::loadNotes()`, `NoteFactory.cpp::reconstructRecord()` | C1, C2, C6 |
| FR-4a | Missing storage file at startup → empty collection, no error | `FileStorage.loadNotes` | `FileStorage.cpp::loadNotes()` — ifstream open check; returns empty vector if absent | C3 |
| FR-4b | Corrupt file → quarantine with timestamp; per-record skip for partial corruption | `FileStorage.loadNotes` | `FileStorage.cpp::loadNotes()` — JSON parse exception → rename to `.quarantine.<ts>`; per-record try/catch | C4, C5 |
| FR-5 | SecureNote passphrase gate; plaintext zeroed after use; decryption buffer zeroed | `SecureNote.lock`, `SecureNote.unlock` | `SecureNote.cpp::lock()` — encrypt + std::fill plaintext; `::unlock()` — decrypt + std::fill buffer | B1, B2, B3, B4, B5 |
| FR-5a | Typed error codes: WRONG_PASSPHRASE vs INVALID_INPUT | `SecureNote.unlock` | `SecureNote.cpp::unlock()` — `std::variant<std::string, SecureNoteError>` return type | B2, B3, B4 |
| FR-6 | Case-insensitive title substring search; empty query returns all; no file I/O | `NoteManager.searchByTitle` | `NoteManager.cpp::searchByTitle()` — std::transform + std::tolower; returns `vector<pair<UUID,string>>` | D1, D2, D3, D4, D5 |
| NFR-1 | O(1) average UUID lookup via unordered_map | `NoteManager.notes_` | `NoteManager.h` — `std::unordered_map<UUID, std::unique_ptr<Note>> notes_` | A1 |
| NFR-2 | No undefined behavior under boundary inputs | All public methods | Input validation in NoteFactory, NoteManager, SecureNote; nullptr returns instead of throws | A7, A12 |
| NFR-3 | VersionHistory populated on every body edit; title edits do not append | `VersionHistory.addEntry`, `NoteManager.editBody` | `VersionHistory.cpp::addEntry()`, called from `NoteManager.cpp::editBody()` after mutation | E1 |
| NFR-4 | External library justified in planning/library-decision.md before use | — | `planning/library-decision.md` — entries for nlohmann/json v3.11.3 and OpenSSL 3.6.1 | — |
| SPR-1 | No plaintext in any JSON file, log, or temp file; buffers zeroed | `SecureNote.lock`, `SecureNote.unlock`, `FileStorage.saveAll` | `SecureNote.cpp` — std::fill on local plaintext/decrypt buffers; `FileStorage.cpp` — hex-encoded ciphertext field only, no body field for SecureNote | A14, B5, B6 |
| SPR-2 | EncryptionEngine is pure abstract; no concrete crypto in public APIs | `EncryptionEngine`, `StorageInterface` | `EncryptionEngine.h` — pure virtual; `SecureNote.h` holds `EncryptionEngine&` not `AESEngine&`; tests use MockEncryptionEngine | B1–B5 |
| SPR-3 | Concurrent access undefined in v1.0; documented in README | — | `README.md` — Known Limitations section | — |

---

## Implementation Location Reference

| Class | File |
|---|---|
| `Note` (abstract base) | `src/model/Note.h`, `src/model/Note.cpp` |
| `TextNote` | `src/model/TextNote.h`, `src/model/TextNote.cpp` |
| `VoiceNote` | `src/model/VoiceNote.h`, `src/model/VoiceNote.cpp` |
| `SecureNote` | `src/model/SecureNote.h`, `src/model/SecureNote.cpp` |
| `VersionHistory` | `src/model/VersionHistory.h`, `src/model/VersionHistory.cpp` |
| `VersionEntry` | `src/model/VersionEntry.h` |
| `NoteFactory` | `src/controller/NoteFactory.h`, `src/controller/NoteFactory.cpp` |
| `NoteManager` | `src/controller/NoteManager.h`, `src/controller/NoteManager.cpp` |
| `StorageInterface` | `src/storage/StorageInterface.h` |
| `FileStorage` | `src/storage/FileStorage.h`, `src/storage/FileStorage.cpp` |
| `EncryptionEngine` | `src/encryption/EncryptionEngine.h` |
| `AESEngine` | `src/encryption/AESEngine.h`, `src/encryption/AESEngine.cpp` |
| `CLIView` | `src/view/CLIView.h`, `src/view/CLIView.cpp` |
| CLI entry point | `src/main.cpp` |
| All tests | `test/test_note_creation.cpp` |

---

## Open Items

| Item | Status |
|---|---|
| VoiceNote audio playback | ⚠️ Deferred — audioPath is stored and editable; no audio playback in v1.0 |
| VersionHistory persistence | ⚠️ By design — history is in-memory only; not serialized to JSON |
| Concurrent file access (SPR-3) | ⚠️ By design — v1.0 limitation documented in README |
| Settings command | ⚠️ Stub — `[s]` prints "not yet implemented"; no SettingsManager class |
