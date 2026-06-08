# AstraNotes — Risk Log

> **Purpose:** Documents risks identified during Sprint Zero planning (S0-9).
> Each entry includes likelihood, impact, mitigation strategy, and current status.
> Source: Lab 2.2 Backlog and Sprint Zero Plan.

---

## Risk Entries

### RISK-001 — EncryptionEngine Boundary Undefined Before Persistence Work

| Field | Value |
|---|---|
| **Likelihood** | HIGH |
| **Impact** | HIGH |
| **Category** | Technical |
| **Identified** | Sprint Zero |
| **Status** | ✅ Resolved |

**Description:** If EncryptionEngine.h was not drafted before FileStorage was written,
the persistence layer would need to be rewritten to comply with SPR-1 when the real
interface was eventually defined.

**Mitigation:** Draft EncryptionEngine.h as a pure abstract interface in Sprint Zero
(S0-5). No FileStorage code touching SecureNote may be written until this interface
is reviewed. Gate enforced by Sprint Zero DoD.

**Resolution:** EncryptionEngine.h was completed in Sprint Zero. FileStorage was
implemented in Steps 4–5 using the established interface boundary. SPR-1 and SPR-2
are fully satisfied. AESEngine is only accessible through `EncryptionEngine&` references —
confirmed by grep across all headers outside `src/encryption/`.

---

### RISK-002 — JSON Library C++17 / GTest Linkage Conflict

| Field | Value |
|---|---|
| **Likelihood** | MEDIUM |
| **Impact** | HIGH |
| **Category** | Technical / Build |
| **Identified** | Sprint Zero |
| **Status** | ✅ Resolved |

**Description:** An untested JSON library could introduce build or linkage failures
that stall Sprint 1.

**Mitigation:** S0-6 requires a build-confirmed linkage check before adopting any
library. If the candidate fails, the fallback is a minimal hand-rolled JSON serializer
scoped to the AstraNotes note schema, documented in planning/library-decision.md.

**Resolution:** nlohmann/json v3.11.3 confirmed compatible with C++17 and Google Test
v1.14.0. Both `cmake --build` and `ctest` exit 0. Full justification in
`planning/library-decision.md`.

---

### RISK-003 — NoteManager Ownership Semantics Chosen Incorrectly

| Field | Value |
|---|---|
| **Likelihood** | LOW |
| **Impact** | HIGH |
| **Category** | Design |
| **Identified** | Sprint Zero |
| **Status** | ✅ Resolved |

**Description:** If shared ownership was required but unique_ptr was used, the ownership
model would need to be redesigned — a costly change after integration.

**Mitigation:** unique_ptr is the initial choice for single ownership. Any change to
shared ownership is an explicit design decision documented in the relevant header with
rationale. The unique_ptr choice is traceable in NoteManager.h.

**Resolution:** unique_ptr throughout. No shared ownership needed. grep confirms no
raw new/delete or shared_ptr in `src/`. RAII enforced. SecureNote uses an injected
`EncryptionEngine&` reference (not owned) per SPR-2.

---

### RISK-004 — Scope Creep From Extensibility Points

| Field | Value |
|---|---|
| **Likelihood** | MEDIUM |
| **Impact** | MEDIUM |
| **Category** | Process |
| **Identified** | Sprint Zero |
| **Status** | ✅ Resolved |

**Description:** AstraNotes has natural extension points (new note types, plugin
interfaces). Uncontrolled scope additions could destabilize the architecture.

**Mitigation:** No feature enters the backlog without a charter requirement mapping
or professor-directed change. S0-10 sprint review checks backlog for drift.
ARCHITECTURE.md lists explicitly prohibited classes (SettingsManager, ProfileService,
SearchIndex, NoteRepository, Logger).

**Resolution:** No undocumented classes introduced. All 10 build steps completed
without scope additions beyond requirement-driven UX improvements (help command,
per-mutation persistence, VoiceNote title/audioPath editing). CLAUDE.md hard
constraint "Do not invent classes" enforced throughout.

---

### RISK-005 — OpenSSL Availability Across Platforms

| Field | Value |
|---|---|
| **Likelihood** | MEDIUM |
| **Impact** | MEDIUM |
| **Category** | Technical / Deployment |
| **Identified** | Step 7 |
| **Status** | ⚠️ Partially mitigated |

**Description:** OpenSSL is a system-installed library. It may not be present by
default on all target platforms, causing the build to fail or AESEngine to be skipped.

**Mitigation:** CMakeLists.txt uses `find_package(OpenSSL QUIET)` with a Homebrew path
hint for macOS. README documents installation instructions for macOS, Linux, and Windows.
Build fails gracefully with a clear error if OpenSSL is absent.

**Resolution:** Verified on macOS with Homebrew OpenSSL 3.6.1. Linux and Windows not
verified — these remain open risk items. Installation instructions added to README.
SPR-3 concurrent access limitation documented in Known Limitations.
