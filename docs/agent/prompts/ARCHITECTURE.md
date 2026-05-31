# AstraNotes — Architecture

<!-- CHANGELOG
- Removed lastModifiedAt_ from Note member table: field does not yet exist in Note.h (added in Step 2)
- Removed durationSec_ from VoiceNote member table: field does not exist in VoiceNote.h
- Removed getLastModifiedAt() from Note public interface: method does not yet exist (added in Step 2)
- Removed serialize() from Note public interface: method does not exist in Note.h
- Corrected NoteManager.remove() return type: currently void, changes to Status in Step 3
- Clarified NullStorage: inline struct, not a separate class file
- Added "(Step 2)" annotations to forward-looking Note members and methods
- Added "(Step 3)" annotation to remove() Status return
-->

> **Purpose of this document:** The definitive structural contract for AstraNotes. Every class name,
> ownership relationship, and interface boundary in the codebase must match what is described here.
> If code diverges from this document, the document is the truth — fix the code, not the document.
> If a new class is genuinely needed, add it here with rationale before writing any code.

---

## MVC Layer Map

```
┌─────────────────────────────────────────────────────┐
│  VIEW                                               │
│  CLIView  («boundary»)                              │
│  - renderNoteList()                                 │
│  - renderNote(uuid)                                 │
│  - promptInput(prompt)                              │
│  Depends on NoteManager. Never touches Note directly│
└────────────────────────┬────────────────────────────┘
                         │ uses
┌────────────────────────▼────────────────────────────┐
│  CONTROLLER                                         │
│  NoteManager                                        │
│  - Owns: map<UUID, unique_ptr<Note>>                │
│  - Holds (injected): NoteFactory&                   │
│  - Holds (injected): StorageInterface&              │
│  NoteFactory  (stateless)                           │
│  - create(type, title) → unique_ptr<Note>           │
│  - reconstructRecord(json) → unique_ptr<Note>       │
└──────┬──────────────────────────┬───────────────────┘
       │ owns                     │ uses interface
┌──────▼──────────┐    ┌──────────▼───────────────────┐
│  MODEL          │    │  INTERFACES                  │
│  Note (abstract)│    │  StorageInterface (pure virt)│
│  TextNote       │    │    saveNote(Note&)            │
│  VoiceNote      │    │    loadNotes() →              │
│  SecureNote─────┼────┼──► vector<unique_ptr<Note>>  │
│  VersionHistory │    │                              │
│  VersionEntry   │    │  EncryptionEngine (pure virt)│
└─────────────────┘    │    encrypt(plain, pass)       │
                       │    decrypt(cipher, pass)      │
                       └──────────────────────────────┘
```

---

## Class Reference

### Note  *(abstract base — `src/model/`)*
**Responsibility:** Abstract base for all note types. Owns UUID, title, timestamps, and VersionHistory.

| Member | Type | Ownership | Notes |
|---|---|---|---|
| `uuid_` | `UUID` (= `std::string`) | value | Set once at construction, never changed |
| `title_` | `std::string` | value | 1–255 non-whitespace chars |
| `createdAt_` | `std::time_t` | value | Set at construction |
| `lastModifiedAt_` | `std::time_t` | value | **Not yet in code — added in Step 2.** Updated on every edit operation. |
| `history_` | `unique_ptr<VersionHistory>` | owned | 1-to-1 composition; lifetime tied to Note |

**Public interface (current):** `getUUID()`, `getTitle()`, `getCreatedAt()`,
`getVersionHistory()`, `getType()` (pure virtual).

**Public interface additions (Step 2):** `getLastModifiedAt()`.

**Note:** `serialize()` is not currently defined on Note. Serialization is handled by FileStorage
using nlohmann/json directly — not through a virtual method on Note.

**Traceability:** FR-1 (refined) | UML: Note

---

### TextNote  *(extends Note — `src/model/`)*
**Responsibility:** Stores plain Markdown body text.

| Member | Type | Notes |
|---|---|---|
| `body_` | `std::string` | Empty by default |

**Public interface:** `getType()` → `"text"`, `getBody()`, `setBody(string)`.

**Traceability:** FR-1, FR-2 | UML: TextNote

---

### VoiceNote  *(extends Note — `src/model/`)*
**Responsibility:** Stores audio file path. Duration and transcription are future features.

| Member | Type | Notes |
|---|---|---|
| `audioPath_` | `std::string` | Always empty this sprint |

**Note:** `durationSec_` is not currently in `VoiceNote.h`. It is a planned field but has not been
implemented. Do not add it until a build step explicitly calls for it.

**Public interface:** `getType()` → `"voice"`, `getAudioPath()`.

**Traceability:** FR-1 | UML: VoiceNote

---

### SecureNote  *(extends Note — `src/model/`)*
**Responsibility:** Holds AES-256-GCM ciphertext only. Plaintext never persisted or stored.

| Member | Type | Ownership | Notes |
|---|---|---|---|
| `ciphertext_` | `std::vector<std::byte>` | value | **Never** `std::string`. SPR-1. |
| `engine_` | `EncryptionEngine&` | **injected** (not owned) | SPR-2 |

**Public interface (current):** `getType()` → `"secure"`, `getCiphertext()`.

**Public interface additions (Step 6):** `lock(plaintext, passphrase)` → `Status`,
`unlock(passphrase)` → `std::variant<std::string, SecureNoteError>`,
`getCiphertextMutable()` → `std::vector<std::byte>&` (needed for zeroing in delete).

**Key invariant (SPR-1):** Plaintext is a stack variable inside `lock()`/`unlock()`. It is zeroed
before the function returns. It never touches any member variable.

**Traceability:** FR-5, SPR-1, SPR-2 | UML: SecureNote

---

### NoteManager  *(controller — `src/controller/`)*
**Responsibility:** Owns the note collection. Orchestrates create/search/persist.

| Member | Type | Ownership |
|---|---|---|
| `notes_` | `unordered_map<UUID, unique_ptr<Note>>` | **owned** (sole authoritative store) |
| `factory_` | `NoteFactory&` | **injected** (not owned) |
| `storage_` | `StorageInterface&` | **injected** (not owned) |

**Public interface:**
- `add(unique_ptr<Note>)` — transfers ownership; throws on UUID collision
- `remove(UUID)` → **currently `void`; changes to `Status` in Step 3** (`Status::OK` if deleted, `Status::NOT_FOUND` if absent)
- `findByUUID(UUID) const` → `const Note*` (nullptr if not found)
- `searchByTitle(string) const` → `vector<string>` (case-insensitive, no I/O) — **fully implemented**
- `persistAll() const` — delegates to `storage_` (stub until Step 4)
- `loadAll()` — delegates to `storage_`; uses NoteFactory for reconstruction (stub until Step 4)
- `getNotes() const` → `const unordered_map<UUID, unique_ptr<Note>>&`

**Edit methods (Step 2):**
- `editTitle(UUID, string)` → `Status`
- `editBody(UUID, string)` → `Status`

**NFR-1:** `unordered_map` gives O(1) average UUID lookup. `std::map` is prohibited.

**Traceability:** NFR-1, FR-1, FR-2, FR-3, FR-4, FR-6 | UML: NoteManager

---

### NoteFactory  *(stateless — `src/controller/`)*
**Responsibility:** Constructs and reconstructs Note subtypes. The only place UUIDs are generated.

**Public interface:**
- `create(type, title)` → `unique_ptr<Note>` — validates title, generates UUID, stamps timestamps
- `reconstructRecord(json)` → `unique_ptr<Note>` — **currently a stub that throws**; implemented in Step 4

**Stateless:** No member variables. Thread-safe by construction.

**Traceability:** FR-1, FR-4 | UML: NoteFactory

---

### VersionHistory  *(owned by Note — `src/model/`)*
**Responsibility:** Maintains ordered list of VersionEntry snapshots for one note.

| Member | Type | Notes |
|---|---|---|
| `entries_` | `vector<VersionEntry>` | Always empty until Step 9 |

**Public interface (current):** `getEntries() const`.

**Public interface additions (Step 9):** `addEntry(VersionEntry)`.

**Ownership:** 1-to-1 composition inside Note. Never hangs off NoteManager.

**Traceability:** NFR-3 | UML: VersionHistory

---

### VersionEntry  *(struct — `src/model/`)*
**Responsibility:** Immutable snapshot record.

| Field | Type |
|---|---|
| `snapshotBody` | `std::string` |
| `timestamp` | `std::time_t` |

**Traceability:** NFR-3 | UML: VersionEntry

---

### StorageInterface  *(pure abstract — `src/storage/`)*
**Responsibility:** Pure virtual contract for persistence. Enables mock in tests (SPR-2).

**Public interface:**
- `saveNote(const Note&)` — persist single note
- `loadNotes()` → `vector<unique_ptr<Note>>`

**Note on saveAll design question:** FR-4 requires atomic serialization of the entire collection.
The current interface declares `saveNote(const Note&)` per-note. When implementing FileStorage,
resolve whether to add `saveAll(const unordered_map<UUID, unique_ptr<Note>>&)` to this interface
or handle the loop in `NoteManager::persistAll()`. Document the decision.

**Implementors:** `FileStorage` (real — Step 4), `NullStorage` (inline struct in `main.cpp` and
test files — not a separate class file).

**Traceability:** SPR-2 | UML: StorageInterface

---

### FileStorage  *(implements StorageInterface — `src/storage/`)*  *(NOT YET CREATED — Step 4)*
**Responsibility:** Reads/writes `notes.json` via nlohmann/json. Implements quarantine logic.

| Member | Type | Notes |
|---|---|---|
| `filePath_` | `std::string` | Configurable; not hardcoded |

**Key behaviors:**
- Atomic write: write to `notes.json.tmp`, then `std::rename()` to `notes.json`
- Quarantine: rename corrupt file to `<name>.quarantine.<unix_timestamp>`
- Per-record error recovery: skip malformed records, load the rest
- Uses NoteFactory to reconstruct notes from JSON on load

**Traceability:** FR-4, FR-4a, FR-4b, SPR-1 | UML: FileStorage

---

### EncryptionEngine  *(pure abstract — `src/encryption/`)*
**Responsibility:** Pure virtual interface for encrypt/decrypt. Enables mock (SPR-2).

**Public interface:**
- `encrypt(plaintext, passphrase)` → `vector<byte>`
- `decrypt(ciphertext, passphrase)` → `string`

**Implementors:** `AESEngine` (real — Step 7), `MockEncryptionEngine` (inline in tests — not a
separate class file).

**Traceability:** SPR-2 | UML: EncryptionEngine

---

### AESEngine  *(implements EncryptionEngine — `src/encryption/`)*  *(NOT YET CREATED — Step 7)*
**Responsibility:** Concrete AES-256-GCM implementation via OpenSSL libcrypto.

**Key constraints:**
- No concrete crypto in any public API (SPR-2)
- Only accessed through `EncryptionEngine&` references
- OpenSSL must be added to CMake link targets when this class is implemented
- `planning/library-decision.md` must include OpenSSL justification before Step 7 begins (NFR-4)

**Traceability:** SPR-1, SPR-2 | UML: AESEngine

---

### CLIView  *(«boundary» — `src/view/`)*
**Responsibility:** Thin presentation layer. No domain logic. Never accesses Note instances directly.

| Member | Type | Ownership |
|---|---|---|
| `manager_` | `NoteManager&` | injected (not owned) |

**Public interface:**
- `renderNoteList() const` — prints index, UUID, type, title for all notes
- `renderNote(uuid) const` — prints full details for one note
- `promptInput(prompt) const` → `std::string` — reads one line, trims whitespace

**MVC constraint:** CLIView calls `manager_.getNotes()` and `manager_.findByUUID()`. It calls only
methods defined on the base `Note` class (`getUUID()`, `getType()`, `getTitle()`, `getCreatedAt()`).
It never calls TextNote-, VoiceNote-, or SecureNote-specific methods.

**Traceability:** FR-1 | UML: CLIView

---

## Key Design Decisions

### Why `unordered_map` instead of `map`
NFR-1 requires O(1) average UUID lookup. `std::map` gives O(log n). The `notes_` field was changed
from `std::map` to `std::unordered_map` in PR #2 (fix/test-cleanup, merged). Side effect: iteration
order is now non-deterministic. A backlog item exists to decide whether stable display order is needed.

### Why interfaces are injected references, not owned pointers
`NoteManager` holds `NoteFactory&` and `StorageInterface&` as references injected at construction time.
`SecureNote` holds `EncryptionEngine&`. This satisfies SPR-2: tests can substitute mock implementations
without touching NoteManager or SecureNote internals. The injector (main.cpp or test fixture) owns the
concrete objects and controls their lifetime.

### Why VersionHistory is owned by Note, not NoteManager
Each note carries its own history as a 1-to-1 composition. This keeps version data co-located with the
note it describes and makes the note lifecycle self-contained. Removing a note automatically removes
its history via `unique_ptr` destruction.

### Why SecureNote ciphertext is `vector<byte>`, not `string`
`std::string` implies text. The ciphertext is raw bytes. Using `vector<byte>` makes it impossible to
accidentally treat ciphertext as a string, prevents silent implicit conversions, and makes SPR-1
compliance visible in the type system.

### Why FileStorage uses atomic rename, not direct write
If the process is killed mid-write, direct writes leave a partially written file. A rename from a
complete temp file is atomic on POSIX systems. This satisfies the FR-4b quarantine requirement and
prevents data loss.

### Why serialize() is not a virtual method on Note
Serialization is the responsibility of FileStorage (the persistence layer), not the model. Making
Note responsible for its own JSON serialization would couple the model to nlohmann/json, violating
the MVC separation principle. FileStorage reads Note's public interface to build JSON records.

---

## What Claude Code Must NOT Invent

The following classes **do not exist** in the AstraNotes design and must never be created:

- `SettingsManager` — settings is a CLI stub only
- `ProfileService` — no profile model exists
- `SearchIndex` — search is a NoteManager method, not a class
- `NoteRepository` — NoteManager is the controller, not a repository pattern
- `Logger` — logging uses `std::cerr` inline; no logging class
- Any class not listed in the Class Reference above
