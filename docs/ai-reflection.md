# AstraNotes AI Reflection Log

## Session 1 — 2026-05-11

### Lab-vs-UML Reconciliation: Profile and Settings

**Conflict identified:** The Week 6 lab specification asks for a "menu / profile / settings / notes workspace" shell. The AstraNotes UML class diagram defines CLIView as a «boundary» class with exactly three methods: `renderNoteList()`, `renderNote()`, and `promptInput()`. There is no `ProfileService`, `SettingsManager`, or any other settings-related class in the validated UML.

**Decision:** Implement profile and settings as a single `[s] settings` menu entry in `main.cpp` that prints `"Settings: not yet implemented."` to stdout. No new class is created.

**Rationale:** Inventing classes not in the UML would introduce untraceable design elements, violate the MVC boundary, and create divergence between the running code and the design document. The stub satisfies the lab deliverable (a menu entry for settings exists and responds to user input) while preserving design integrity and avoiding future rework when a real settings model is eventually specified and added to the UML.

**Traceability:** Lab-vs-UML reconciliation | UML: CLIView (no settings method defined)

---

### Sprint Zero Summary

All S0-1 through S0-10 tasks completed on 2026-05-11. Build system: CMake 3.14+ fetches Google Test v1.14.0 and nlohmann/json v3.11.3 via FetchContent. OpenSSL is detected with `find_package(OpenSSL QUIET)` but linked to no target this sprint; the link step is reserved for AESEngine in a later sprint. Both `cmake --build` and `ctest` exit 0.

---

### AI-Generated Code Hallucination Scan

All class names in generated code — `Note`, `TextNote`, `VoiceNote`, `SecureNote`, `VersionHistory`, `VersionEntry`, `StorageInterface`, `EncryptionEngine`, `NoteFactory`, `NoteManager`, `CLIView` — appear in the UML class diagram. No class was invented. `UUID` is a `std::string` typedef; no external UUID library was invented. All ownership uses `std::unique_ptr`; no raw `new` or `delete` is present. `SecureNote::ciphertext_` is `std::vector<std::byte>` throughout; the type was never weakened to `std::string`. `NullStorage` in `main.cpp` and in tests is an inline implementation of `StorageInterface`, not a new UML class.

---

## Session 2 — 2026-06-07: Audit, Feature Extension, and UX Improvements

### Overview

This session covered the full build arc from post-audit fixes through UX improvements
and CLI wiring. It demonstrates the AI-native practice pattern throughout: the AI proposed
changes, the human reviewed and directed, and human corrections were applied before any
commit was pushed.

---

### Decision: TextNote body invisible after editing

**What the AI identified:** `CLIView::renderNote()` printed UUID, type, title, and
created timestamp only. There was no code path calling `TextNote::getBody()`. A user
who edited a TextNote body and then viewed it saw no change — the body was silently
ignored.

**What the human directed:** Confirmed the gap. Approved Option A: add a virtual
`Note::getBody()` returning `""` by default, overridden in TextNote and VoiceNote,
so CLIView can call it without a type cast.

**Design decision:** `getBody()` returns `std::string` by value (not `const std::string&`)
to allow the base class default to return a temporary. TextNote's existing `const std::string&`
return was changed to `std::string` to make it a proper override with the `override` keyword
enforced by the compiler.

---

### Decision: SecureNote body editing returning "invalid input"

**What the human reported:** Attempting to edit a SecureNote body via `[e]` returned
`"Error: invalid input."` — the same error shown for an invalid field value, giving
no indication that the operation was unsupported for that type.

**What the AI diagnosed:** `NoteManager::editBody()` cast to `TextNote*`; any non-TextNote
returned `INVALID_INPUT`. SecureNote body editing requires a passphrase-based re-lock
flow that didn't exist.

**What the human directed:** Fix it — SecureNote body should be editable via passphrase.

**Design decision:** Added `NoteManager::relockSecureBody(uuid, newBody, passphrase)`
which calls `SecureNote::lock()` with the new content. The CLI edit branch detects
`secure` type before prompting for a new value, then routes to the passphrase flow.
This keeps the controller layer responsible for type-specific routing; `CLIView` remains
type-agnostic.

---

### Decision: VoiceNote editing rejected as "invalid input"

**What the human questioned:** "Shouldn't you be able to edit the title and body/path
to audio just like a text note file?"

**What the AI had assumed:** FR-2 is titled "Edit TextNote" in the spec, so `editTitle`
and `editBody` both cast to `TextNote*` and rejected all other types. The AI had
previously explained this as correct spec behavior.

**Human oversight caught the error:** The spec restriction was an implementation artifact,
not a sound design decision. The title field lives on the `Note` base class, not on
`TextNote`. There is no reason to prevent renaming a VoiceNote.

**Design decision:** Removed the `TextNote*` cast from `editTitle` for non-SecureNote
types. Added `VoiceNote::setAudioPath()` and `VoiceNote::getBody()` (returns `audioPath_`)
so the existing `[e] body` command sets the audio path and `[v]` view displays it.
`editBody` now routes VoiceNote to `setAudioPath` and TextNote to `setBody`. The existing
test that asserted VoiceNote editing returned `INVALID_INPUT` was updated to assert both
edits succeed.

---

### Decision: SecureNote title editing blocked without justification

**What the human questioned:** "Why can't I change the title of a SecureNote?"

**What the AI found:** When fixing VoiceNote, the AI had added an explicit guard
`if (dynamic_cast<const SecureNote*>(...)) return INVALID_INPUT` to `editTitle` with
the comment "title is the only unencrypted identifier." This was incorrect reasoning —
the UUID is the immutable identifier, not the title. The title is plaintext metadata.

**Human oversight caught the error:** The guard was wrong. Renaming a SecureNote is
safe and expected.

**Design decision:** Removed the guard. Title editing now works for all note types.
Body editing for SecureNote still requires the passphrase re-lock flow.

---

### Decision: Notes only persisted on quit

**What the human identified:** `manager.persistAll()` was only called in the `[q]`
quit branch. Any process exit other than a clean quit (crash, Ctrl-C, kill) lost all
unsaved changes.

**Design decision:** Added `persistAll()` calls immediately after every successful
mutation: text/voice create, secure create, edit (both regular and re-lock), and delete.
Failed operations do not write to disk. The quit branch retains its `persistAll()` call
as the definitive final save.

---

### Decision: searchByTitle return type changed from titles to UUID+title pairs

**What the AI proposed:** The existing `searchByTitle` returning `vector<string>` (titles
only) is not actionable when duplicate titles exist. FR-1 explicitly allows two notes
with identical titles. A search result showing two identical strings gives the user no
way to distinguish them for a subsequent `[v]` or `[e]` command.

**Human direction:** Agreed. Change the return type to `vector<pair<UUID, string>>`.
Update all affected tests.

**Design decision:** `searchByTitle` now returns `vector<pair<UUID, string>>`. The
`[/]` CLI command prints `UUID  title` per row, matching the `[l]` list format. Tests
D2 and D3 were updated to destructure pairs. D3 also asserts the UUID is non-empty to
lock in the contract at the test level.

---

### AI-Native Practice Pattern — Summary

The table below shows every significant decision in Session 2, who initiated it, and
what human oversight contributed:

| Decision | Initiator | Human oversight action |
|---|---|---|
| TextNote body invisible | AI (code review) | Confirmed gap; approved fix approach |
| SecureNote body editing | Human (bug report) | Directed implementation of re-lock flow |
| Per-mutation persistence | Human (question) | Identified architectural gap; directed fix |
| Help command | Human (analysis) | Provided full analysis; AI implemented exactly as specified |
| Search CLI wiring | Human (analysis) | Identified unwired feature; directed implementation |
| Search return type | AI (proposal) | Human reviewed tradeoff, agreed, directed update |
| TextNote body at creation | Human (analysis) | Provided analysis; directed implementation |
| VoiceNote editing | Human (correction) | Corrected AI's incorrect "spec-compliant" justification |
| SecureNote title editing | Human (question) | Caught AI-introduced incorrect guard; directed removal |
| Separate commits | Human (direction) | Required logical commit separation; AI replayed changes in order |

---

## Session 3 — 2026-06-08: Documentation Completion

### Overview

The final session completed the documentation layer for the project submission. The AI
audited the repository against the rubric, identified gaps, proposed a plan, received
human confirmation, and executed. All documentation files were created from the actual
source — test counts from the test file, commands from main.cpp, requirements from the
agent documents.

### Human oversight actions

- Reviewed the Phase 2 audit output before approving Phase 3 execution
- Confirmed the commit strategy (one logical commit for all doc files)
- Added lab PDFs independently, then directed AI to commit them
- Reviewed gap analysis (6 gaps) and confirmed all 6 should be addressed
- UML images were committed by the human directly, not by AI direction
