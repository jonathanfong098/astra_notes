# AstraNotes — Audit and Fix Prompt

## Your Role
You are a senior C++17 software engineer performing a full audit of the AstraNotes application.
The application has been built through a 10-step build plan and is considered feature-complete.
Your job is to find every issue, bug, and problem, then fix them one at a time with the developer
verifying each fix before you proceed to the next.

---

## Phase 1 — Read Context (Do This First, Before Anything Else)

Read these files in full before writing a single line of output:

1. `docs/agent/PROJECT_OVERVIEW.md`
2. `docs/agent/REQUIREMENTS.md`
3. `docs/agent/ARCHITECTURE.md`
4. `docs/agent/BUILD_PLAN.md`
5. `docs/agent/TEST_REFERENCE.md`

Then read every source file in the following order:
- `CMakeLists.txt` and `test/CMakeLists.txt`
- `src/model/Note.h` and `Note.cpp`
- `src/model/TextNote.h` and `TextNote.cpp`
- `src/model/VoiceNote.h` and `VoiceNote.cpp`
- `src/model/SecureNote.h` and `SecureNote.cpp`
- `src/model/VersionHistory.h` and `VersionHistory.cpp`
- `src/model/VersionEntry.h`
- `src/controller/NoteFactory.h` and `NoteFactory.cpp`
- `src/controller/NoteManager.h` and `NoteManager.cpp`
- `src/storage/StorageInterface.h`
- `src/storage/FileStorage.h` and `FileStorage.cpp`
- `src/encryption/EncryptionEngine.h`
- `src/encryption/AESEngine.h` and `AESEngine.cpp`
- `src/view/CLIView.h` and `CLIView.cpp`
- `src/main.cpp`
- `test/test_note_creation.cpp`
- `CLAUDE.md`
- `planning/library-decision.md`
- `README.md`

Do not begin Phase 2 until every file above has been read.

---

## Phase 2 — Full Audit

Run the build and tests first:
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
ctest --test-dir build --output-on-failure
```

Report the exact output. Then systematically inspect every dimension below.
For each finding, record it in the format specified at the end of this phase.

---

### Dimension 1 — Correctness

Check every implemented feature against its requirement in `docs/agent/REQUIREMENTS.md`.

**FR-1 / Note Creation:**
- Does `NoteFactory::create()` reject empty titles? Whitespace-only titles? Titles > 255 chars?
- Does it generate RFC 4122 v4 UUIDs (version bits 0100, variant bits 10xx)?
- Does `NoteManager::add()` throw on UUID collision?
- Does NoteFactory return the correct concrete type for "text", "voice", "secure"?

**FR-2 / Edit TextNote:**
- Does `editTitle()` validate the new title against FR-1a rules?
- Does `editBody()` update `lastModifiedAt_` on success?
- Are edits atomic — if title validation fails, is the note completely unchanged?
- Does editing body only leave title unchanged and not re-validate it?
- Does editing a missing UUID return `Status::NOT_FOUND` without throwing?
- Does editing a VoiceNote or SecureNote return `Status::INVALID_INPUT`?

**FR-3 / Delete Note:**
- Does `remove()` return `Status::OK` on success and `Status::NOT_FOUND` for missing UUID?
- Is SecureNote ciphertext zeroed before deallocation?
- Are other notes unaffected by a delete?

**FR-4 / Persistence:**
- Does FileStorage write atomically (temp file then rename)?
- Does `reconstructRecord()` restore UUID, title, type, createdAt, lastModifiedAt correctly?
- Does a missing file return an empty collection with no error?
- Does `persistAll()` and `loadAll()` correctly wire through NoteManager?

**FR-4b / Quarantine:**
- Is a fully corrupt file renamed to `<name>.quarantine.<unix_timestamp>`?
- Are individual malformed records skipped without stopping the load?
- Does a duplicate UUID discard the second record and log the conflict?

**FR-5 / SecureNote Passphrase Gate:**
- Does `lock()` store ciphertext and zero the plaintext local variable after encryption?
- Does `unlock()` return `std::variant<std::string, SecureNoteError>`?
- Does an empty passphrase return `INVALID_INPUT` without calling `engine_.decrypt`?
- Does a passphrase shorter than 8 chars return `INVALID_INPUT`?
- Does a wrong passphrase return `WRONG_PASSPHRASE`?
- Is the decryption buffer zeroed after `unlock()` completes, whether success or failure?

**FR-6 / Search:**
- Is search case-insensitive?
- Does an empty query return the full collection?
- Does search perform no file I/O?

**NFR-1 / O(1) Lookup:**
- Is `notes_` an `std::unordered_map`, not `std::map`?

**NFR-3 / VersionHistory:**
- Does `editBody()` append a VersionEntry with the new body and a timestamp?
- Does `editTitle()` NOT append a VersionEntry?

---

### Dimension 2 — Security

**SPR-1 — No plaintext on disk:**
- Run: `grep -rn "plaintext\|body" src/storage/FileStorage.cpp`
- Verify SecureNote JSON records contain only: uuid, type, title, createdAt, lastModifiedAt, ciphertext
- Verify there is no code path in FileStorage that writes a "body" field for SecureNote

**SPR-2 — Injectable interfaces:**
- Run: `grep -rn "AESEngine\|OpenSSL\|EVP" src/model/ src/controller/ src/storage/ src/view/ --include="*.h"`
- Result must be zero — concrete crypto must not appear in any public API header outside `src/encryption/`
- Verify `SecureNote` holds `EncryptionEngine&`, not `AESEngine&`

**SPR-3 — Concurrent access:**
- Is the concurrent access limitation documented in `README.md`?

**Passphrase handling:**
- Does passphrase length use `.size()` not `strlen()`?
- Is the decryption buffer zeroed with `std::fill` before going out of scope?
- Is the plaintext local variable in `lock()` zeroed after encryption?

**AESEngine integrity:**
- Does `encrypt()` include an IV in the output (IV must be stored alongside ciphertext for decryption)?
- Does `decrypt()` verify the GCM authentication tag before returning plaintext?
- Is the key derived from the passphrase (PBKDF2 or equivalent) rather than used as a raw key?

---

### Dimension 3 — Test Coverage

Run: `ctest --test-dir build --output-on-failure --verbose`

For every test in `docs/agent/TEST_REFERENCE.md`, report:
- Is it implemented in `test/test_note_creation.cpp`?
- Does it pass?
- Does it actually test what the TEST_REFERENCE says it tests (not just compile and pass trivially)?

Check for these specific gaps:
- D1: Does it verify that non-matching notes are excluded (not just that matching ones are included)?
- D5: Does it use `FailOnCallStorage` to prove no I/O occurs?
- A14: Does it verify the ciphertext bytes are all zero (not just that the note was deleted)?
- B5: Does it verify no plaintext is accessible through any public accessor after unlock?
- B6: Does it open and parse the actual JSON file to confirm no plaintext field exists?
- E1: Does it verify both the size of entries AND the content of each entry?

Check for untested behaviors not in TEST_REFERENCE:
- Is there a test for `NoteManager::persistAll()` calling FileStorage correctly?
- Is there a test for `NoteManager::loadAll()` populating the notes map?
- Is there a test for UUID collision in `NoteManager::add()`?
- Is there a test for editing a VoiceNote returning `INVALID_INPUT`?

---

### Dimension 4 — Code Quality

**Traceability comments:**
- Run: `grep -rL "Traceability" src/ --include="*.h"` — any result is a missing comment
- Run: `grep -rL "Traceability" src/ --include="*.cpp"` — any result is a missing comment
- Every header and every non-trivial function must have `// Traceability: <REQ-ID> | UML: <ClassName.method>`

**Raw pointer safety:**
- Run: `grep -rn "\bnew \b\|\bdelete\b" src/ --include="*.cpp" --include="*.h"`
- Result must be zero

**C++17 compliance:**
- Run: `grep -rn "std::format\|std::ranges\|co_await\|co_yield\|concept\b\|requires\b" src/`
- Result must be zero (these are C++20 features)

**Smart pointer discipline:**
- Are there any raw owning pointers in any public API?
- Is there any `shared_ptr` usage without a documented rationale comment?

**Error handling consistency:**
- Does every function that can fail return `Status` or throw `std::invalid_argument` consistently?
- Are there any functions that silently swallow errors or return without indication of failure?

**Const correctness:**
- Are read-only methods marked `const`?
- Are parameters that are not modified passed by const reference?

**Include hygiene:**
- Are there any unnecessary `#include` directives?
- Are there any missing includes that only work due to transitive inclusion?

---

### Dimension 5 — Architectural Integrity

**MVC boundaries:**
- Does `CLIView` access any Note subtype method directly (TextNote::getBody, SecureNote::getCiphertext)?
  Run: `grep -n "getBody\|getCiphertext\|getCiphertextMutable\|lock\|unlock" src/view/CLIView.cpp`
- Does `NoteManager` perform any direct filesystem I/O?
  Run: `grep -n "fstream\|ifstream\|ofstream\|FILE\|fopen" src/controller/NoteManager.cpp`
- Does `FileStorage` access `NoteManager` or `CLIView`?
  Run: `grep -n "NoteManager\|CLIView" src/storage/FileStorage.cpp`

**Class responsibility drift:**
- Does any class do more than its stated responsibility in `docs/agent/ARCHITECTURE.md`?
- Has any class been invented that is not in the ARCHITECTURE.md Class Reference?
  Run: `grep -rn "^class " src/ --include="*.h"` and compare against the documented list

**Ownership integrity:**
- Does `NoteManager` still use `unordered_map<UUID, unique_ptr<Note>>`?
- Does `SecureNote` still hold `EncryptionEngine&` (reference, not pointer)?
- Does `NoteManager` still hold `NoteFactory&` and `StorageInterface&` (references, not owned)?

**Interface purity:**
- Does `StorageInterface` have any non-virtual, non-destructor members?
- Does `EncryptionEngine` have any non-virtual, non-destructor members?

---

### Finding Format

Record every issue found in this exact format:

```
FINDING-001
Severity: CRITICAL | HIGH | MEDIUM | LOW
Dimension: Correctness | Security | Test Coverage | Code Quality | Architecture
File: <filename>:<line number if applicable>
Description: <one sentence describing the problem>
Requirement: <FR/NFR/SPR ID if applicable>
Fix: <one sentence describing what needs to change>
```

Severity definitions:
- **CRITICAL** — incorrect behavior, data loss, security violation, or crash risk
- **HIGH** — requirement not satisfied, test missing for implemented behavior, or ownership error
- **MEDIUM** — traceability missing, const correctness, include hygiene, minor API inconsistency
- **LOW** — naming, formatting, documentation gap with no behavioral impact

---

## Phase 3 — Findings Report

After completing all five audit dimensions, produce a consolidated report:

```
=== ASTRANOTES AUDIT REPORT ===

Build status: PASS / FAIL
Test results: X/Y passing

CRITICAL findings: N
HIGH findings: N
MEDIUM findings: N
LOW findings: N
Total findings: N

--- CRITICAL ---
[list all CRITICAL findings]

--- HIGH ---
[list all HIGH findings]

--- MEDIUM ---
[list all MEDIUM findings]

--- LOW ---
[list all LOW findings]

=== END REPORT ===
```

Then state:
> "Audit complete. I found N issues. Ready to begin fixes starting with the highest severity.
> Please confirm to proceed."

**STOP HERE. Do not begin any fixes until Jonathan confirms.**

---

## Phase 4 — Fix Loop

After Jonathan confirms, work through findings in order: CRITICAL first, then HIGH, then MEDIUM,
then LOW. For each finding:

### Fix Protocol

1. **Announce the fix:**
   ```
   === FIXING FINDING-XXX ===
   Severity: <severity>
   File: <file>
   Problem: <one sentence>
   Approach: <one sentence describing what you will change>
   ```

2. **Make the change.** One finding at a time. Do not batch multiple fixes.

3. **Verify immediately after every fix:**
   ```bash
   cmake --build build
   ctest --test-dir build --output-on-failure
   ```

4. **Report the result:**
   ```
   === FINDING-XXX RESULT ===
   Build: PASS / FAIL
   Tests: X/Y passing
   Tests added: <list any new test names>
   Tests changed: <list any modified tests>
   Fix confirmed: YES / NO
   ```
   If `ctest` fails after a fix, diagnose and repair before moving on.
   Never leave the build broken between findings.

5. **STOP and state:**
   > "FINDING-XXX is resolved. Please verify and confirm to continue to FINDING-YYY."

6. **Wait for Jonathan's confirmation** before proceeding to the next finding.

### Rules for the Fix Loop

- **One fix per confirmation.** Never fix two findings in one step.
- **Never break the build.** If a fix causes a new failure, fix the regression before stopping.
- **New tests count as fixes.** If a finding is "behavior X has no test," the fix is writing
  the test. The test must pass before stopping.
- **Do not refactor beyond the finding.** Fix exactly what is described. If you notice a
  related issue while fixing, record it as a new FINDING and address it in sequence.
- **Traceability required.** Every new or modified function must have a traceability comment.
- **No C++20 features.** C++17 strictly throughout.
- **No raw new/delete.** No raw owning pointers introduced.

---

## Phase 5 — Final Verification

After all findings are resolved, run the complete final check:

```bash
# Full build and test
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
ctest --test-dir build --output-on-failure

# Safety checks
grep -rn "\bnew \b\|\bdelete\b" src/ --include="*.cpp" --include="*.h"
grep -rn "std::map" src/controller/ --include="*.h"
grep -rn "plaintext\|\"body\"" src/storage/FileStorage.cpp
grep -rn "AESEngine\|OpenSSL\|EVP" src/model/ src/controller/ src/storage/ src/view/ --include="*.h"
grep -rn "std::format\|std::ranges\|co_await\|concept\b\|requires\b" src/

# Traceability coverage
grep -rL "Traceability" src/ --include="*.h"
grep -rL "Traceability" src/ --include="*.cpp"
```

Then produce a final summary:

```
=== ASTRANOTES POST-AUDIT SUMMARY ===

Findings resolved: N/N
Build: PASS
Tests: X/X passing
All safety checks: CLEAN / [list any remaining issues]

Application status: PRODUCTION READY / NEEDS FURTHER WORK
=== END SUMMARY ===
```
