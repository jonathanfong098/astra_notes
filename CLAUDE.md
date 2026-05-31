# AstraNotes — Claude Code Session Context

## Read These First (Every Session)
1. docs/agent/PROJECT_OVERVIEW.md  — current state, hard rules, what's done
2. docs/agent/REQUIREMENTS.md      — authoritative requirements
3. docs/agent/ARCHITECTURE.md      — class structure, ownership, MVC boundaries
4. docs/agent/BUILD_PLAN.md        — what to build next and when to stop
5. docs/agent/TEST_REFERENCE.md    — test specifications before writing any test

## Hard Constraints (non-negotiable)
- C++17 strictly. No C++20 features.
- unique_ptr for all ownership. No raw new/delete. No raw owning pointers in public API.
- MVC separation: CLIView (View), NoteManager (Controller), Note hierarchy (Model).
- Google Test required. cmake --build and ctest must both exit 0 before any step is complete.
- SecureNote: std::vector<std::byte> for ciphertext only. No plaintext in any member, log, or file.
- EncryptionEngine and StorageInterface are pure abstract interfaces. No concrete crypto in public APIs.

## Traceability Rule
Every class header and every implemented function must include:
// Traceability: <REQ-ID> | UML: <ClassName.methodName>