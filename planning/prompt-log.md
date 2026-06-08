# AstraNotes Prompt Log

## Entry 1

- **Date:** 2026-05-11
- **Task:** Sprint Zero bootstrap (S0-1 through S0-10) + Note Creation slice (US-01 / FR-1)
- **Prompt summary:** Self-contained Claude Code prompt covering: repository scaffold, root and test CMakeLists.txt with Google Test v1.14.0 and nlohmann/json v3.11.3 via FetchContent, all interface headers (Note abstract base, TextNote/VoiceNote/SecureNote stubs, VersionHistory/VersionEntry stubs, StorageInterface pure abstract, EncryptionEngine pure abstract, NoteFactory stateless factory, NoteManager controller, CLIView boundary), NoteFactory.create implementation with pseudo-UUID generation, NoteManager.add + findByUUID implementation, three GTest cases (happy path, empty-title rejection, UUID uniqueness), main.cpp CLI loop with [n]/[l]/[s]/[q] menu, traceability comments throughout, DoD checklist, and lab-vs-UML reconciliation note.
- **Output summary:** All Sprint Zero tasks S0-1 through S0-10 completed. Note Creation slice (US-01 / FR-1) implemented with AC-01-1, AC-01-2, AC-01-3 verified by GTest. cmake --build and ctest both exit 0.
- **Decision:** Profile and Settings implemented as a single `[s] settings` CLI stub (prints "Settings: not yet implemented.") in main.cpp. No SettingsManager or ProfileService class created.
- **Rationale:** The Week 6 lab deliverable asks for a menu/profile/settings/notes workspace shell. The validated AstraNotes UML defines CLIView with only three methods (renderNoteList, renderNote, promptInput). Inventing new classes not in the UML would contradict the design contract and create untraceable divergence. The stub satisfies the lab requirement (a settings menu entry is present) while preserving MVC architecture integrity.

---

## Entry 2

- **Date:** 2026-05-25 to 2026-05-31
- **Task:** Build Steps 1–10 — full feature implementation from Edit TextNote through Final Audit
- **Prompt used:** `docs/agent/prompts/AUDIT_AND_FIX.md` — a structured multi-phase audit prompt covering correctness, security, test coverage, code quality, and architectural integrity across all five dimensions.
- **Steps completed in this session:**
  - Step 1: Added tests D4 (EmptyQuery_ReturnsAllNotes), A4 (RejectsTitleExceeding255Chars), A7 (FindByUUIDReturnsNullForMissingUUID)
  - Step 2: Implemented FR-2 — editTitle / editBody with atomic semantics, lastModifiedAt, Status return
  - Step 3: Implemented FR-3 — delete with Status return, SecureNote ciphertext zeroing
  - Step 4: Implemented FR-4 — FileStorage saveAll/loadNotes with atomic write and NoteFactory.reconstructRecord
  - Step 5: Implemented FR-4b — quarantine logic and per-record skip
  - Step 6: Implemented FR-5 — SecureNote lock/unlock with MockEncryptionEngine in tests
  - Step 7: Implemented AESEngine with real AES-256-GCM via OpenSSL; added B6 integration test
  - Step 8: Wired all features into CLI; replaced NullStorage with FileStorage in main.cpp
  - Step 9: Implemented VersionHistory.addEntry; called from editBody; test E1 added
  - Step 10: Final audit — grep checks, traceability sweep, DoD sign-off
- **Output summary:** All 11 FINDING-* issues identified by audit resolved. 27/27 tests passing at step completion.
- **Key human oversight decisions:**
  - Human reviewed each FINDING before confirming the fix
  - Human rejected one proposed approach for A14 (post-deallocation byte check was UB) and accepted the direct-zeroing verification pattern instead
  - Human confirmed library-decision.md OpenSSL entry before Step 7 began (NFR-4 gate)

---

## Entry 3

- **Date:** 2026-06-07
- **Task:** Feature extension, UX improvements, and bug fixes following live application testing
- **Prompting pattern:** Conversational — human ran the app, reported observed behavior, and directed specific changes. No structured prompt file used.
- **Changes made (in order):**
  1. `renderNote()` gap: TextNote body never displayed — added virtual `Note::getBody()`, overridden in TextNote and VoiceNote
  2. SecureNote body editing: added `NoteManager::relockSecureBody()` and CLI passphrase re-lock flow
  3. Per-mutation persistence: `persistAll()` added after every successful create, edit, and delete
  4. Help command: `[h]` handler added; unknown-command message updated to point to it
  5. Search CLI wiring: `[/]` command added; `searchByTitle` return type changed to `vector<pair<UUID,string>>`
  6. TextNote body prompt at creation: text creation branch prompts for body before `manager.add()`
  7. VoiceNote editing: `setAudioPath()` added; `editTitle` and `editBody` extended to handle VoiceNote
  8. SecureNote title editing: AI-introduced incorrect guard removed; all note types can now rename
- **Commit strategy:** Human directed that changes be split into logically separate commits. AI stashed all changes and replayed them incrementally, committing after each logical unit.
- **Human oversight actions:** Human caught two AI errors (VoiceNote editing incorrectly justified as spec-compliant; SecureNote title guard incorrectly introduced). Both were reversed on human direction.

---

## Entry 4

- **Date:** 2026-06-08
- **Task:** Repository documentation completion for final project submission
- **Prompt used:** Structured documentation completion prompt (pasted inline by human) specifying exact files to create/update, exact sections to add, and explicit constraints (no src/ changes, accurate content only, test counts from actual file).
- **Files created:** `docs/testing-strategy.md`, `docs/test-results.txt`, `planning/traceability.md`, `planning/risk-log.md`
- **Files updated:** `README.md` (added Prerequisites, Features, Commands, Repository Structure, Lab Submissions)
- **Human oversight actions:**
  - Human reviewed Phase 2 audit output before approving Phase 3 execution
  - Human added lab PDFs independently (not AI-directed)
  - Human confirmed 6-point gap analysis before directing remaining documentation work
  - Human confirmed UML images were already committed; AI adjusted plan accordingly
