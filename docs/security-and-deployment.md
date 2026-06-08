# AstraNotes — Security, Deployment, and Maintenance Notes

---

## Security Design

### Encryption (FR-5, SPR-1, SPR-2)

AstraNotes uses **AES-256-GCM** for SecureNote content via OpenSSL libcrypto. AES-256-GCM
provides authenticated encryption: it simultaneously encrypts content and produces a
16-byte authentication tag that detects tampering or corruption.

**Key derivation:** Passphrases are not used as raw AES keys. Keys are derived using
**PBKDF2-SHA256** with a random 16-byte salt, stretching the passphrase into a 256-bit
key. The salt is stored alongside the ciphertext.

**Wire format stored in JSON:**
```
SALT (16 bytes) | IV (12 bytes) | TAG (16 bytes) | CIPHERTEXT (variable)
```
This is hex-encoded as a single string in the `"ciphertext"` JSON field. No other
field for SecureNote content exists in any JSON file.

**Plaintext handling (SPR-1):**
- Plaintext is a local stack variable inside `SecureNote::lock()` only
- It is zeroed with `std::fill` before `lock()` returns
- The decryption buffer in `unlock()` is zeroed with `std::fill` whether decryption
  succeeds or fails
- Plaintext never touches any class member variable

**Passphrase minimum (FR-5):** 8 characters, enforced by `.size()` not `strlen()` to
correctly handle null bytes in passphrases.

**Typed error codes (FR-5a):** `unlock()` returns `std::variant<std::string, SecureNoteError>`.
`SecureNoteError::WRONG_PASSPHRASE` and `SecureNoteError::INVALID_INPUT` are distinct
so callers can give accurate feedback without exposing whether a passphrase exists.

---

### Interface Injection (SPR-2)

`EncryptionEngine` is a pure abstract class. `SecureNote` holds `EncryptionEngine&` —
an injected reference, not an `AESEngine` pointer. This means:

- Unit tests substitute `MockEncryptionEngine` without linking OpenSSL
- The concrete crypto implementation is swappable without changing any public API
- `grep -rn "AESEngine" src/model/ src/controller/ src/storage/ src/view/` returns zero —
  `AESEngine` is never referenced outside `src/encryption/` and `src/main.cpp`

The same pattern applies to `StorageInterface` / `FileStorage`.

---

### No Plaintext on Disk Guarantee (SPR-1)

SecureNote JSON records contain exactly:
```json
{
  "uuid": "...",
  "type": "secure",
  "title": "...",
  "createdAt": 1234567890,
  "lastModifiedAt": 1234567890,
  "ciphertext": "<hex-encoded SALT|IV|TAG|CIPHERTEXT>"
}
```
There is no `"body"` or `"plaintext"` field. `FileStorage::saveAll()` uses a
`dynamic_cast<const SecureNote*>` branch that writes only the ciphertext hex string.
Test B6 (`FileStorage_CiphertextOnly`) opens and inspects the raw JSON to assert
no plaintext field exists.

---

### Known Security Limitations

| Limitation | Status |
|---|---|
| Concurrent file access by two instances is undefined behavior | ⚠️ By design — v1.0 (SPR-3) |
| No file locking or atomic multi-process coordination | ⚠️ Not implemented |
| Passphrase is not memory-locked (`mlock`) | ⚠️ Not implemented — OS may page passphrase to swap |
| No key stretching iteration count tuning | ⚠️ PBKDF2 iteration count is fixed; not configurable |
| VoiceNote audio content is not encrypted | ⚠️ Only the path string is stored; audio files are unprotected |

---

## Deployment

### Requirements

| Dependency | Version | Source |
|---|---|---|
| CMake | 3.14+ | System package manager |
| C++17 compiler | Apple Clang / GCC 9+ / MSVC 2019+ | System |
| OpenSSL | 3.x | Homebrew / apt / choco (see README) |
| Google Test | 1.14.0 | Auto-fetched via FetchContent at configure time |
| nlohmann/json | 3.11.3 | Auto-fetched via FetchContent at configure time |

### Build

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

For debug builds with AddressSanitizer:
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
ctest --test-dir build --output-on-failure
```

### Data Storage

Notes are stored at `~/.astranotes/notes.json`. The directory is created automatically
on first run. The file path is configurable via `FileStorage` constructor — `main.cpp`
resolves it from `$HOME`.

**Atomic write:** All saves write to `notes.json.tmp` first, then rename to `notes.json`.
On POSIX systems `rename()` is atomic — a crash mid-write leaves the previous file intact.

**Corrupt file recovery:** If `notes.json` is unparseable at startup, it is renamed to
`notes.json.quarantine.<unix_timestamp>` and the application starts with an empty
collection. If individual records are malformed inside an otherwise valid file, those
records are skipped and all healthy records load normally.

### Platforms

| Platform | Status | Notes |
|---|---|---|
| macOS (Apple Silicon / Intel) | ✅ Verified | Homebrew OpenSSL 3.x required |
| Linux | ⚠️ Expected to work | `libssl-dev` required; not CI-verified |
| Windows | ⚠️ Expected to work | MSVC 2019+ and OpenSSL installer required; not verified |

---

## Maintenance and Extension

### Adding a New Note Type

AstraNotes is designed to accommodate new note types without modifying existing classes:

1. **Create the model class** in `src/model/` extending `Note`. Add `getType()`, any
   type-specific fields, getters, and setters. Add traceability comments.

2. **Register in NoteFactory** — add a branch in `NoteFactory::create()` for the new
   type string, and add a branch in `NoteFactory::reconstructRecord()` to deserialize it.

3. **Handle in FileStorage** — add a branch in the JSON serialization logic in
   `FileStorage::saveAll()` to write type-specific fields.

4. **Override `getBody()`** on the base `Note` class if the new type has displayable
   content — `CLIView::renderNote()` will display it automatically.

5. **Handle in `NoteManager::editBody()`** if the new type has an editable content field —
   add a `dynamic_cast` branch routing to the appropriate setter.

6. **Write tests** covering creation, serialization round-trip, and any type-specific behavior.

### Modifying the Encryption Algorithm

`EncryptionEngine` is a pure abstract interface. To swap the encryption algorithm:

1. Create a new class implementing `EncryptionEngine` in `src/encryption/`
2. Update `main.cpp` to instantiate the new engine instead of `AESEngine`
3. Update `planning/library-decision.md` per NFR-4 before any new library is used

No changes to `SecureNote`, `NoteManager`, `FileStorage`, or any test using `MockEncryptionEngine`
are required.

### Running the Test Suite

```bash
# All tests
ctest --test-dir build --output-on-failure

# Specific suite
ctest --test-dir build --output-on-failure -R SecureNote
ctest --test-dir build --output-on-failure -R FileStorageTest

# Verbose
ctest --test-dir build --output-on-failure --verbose
```

All 33 tests must pass before any change is considered complete.
