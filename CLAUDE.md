# AstraNotes — Claude Code Session Context

## Project
C++17 CLI note-taking application. MVC architecture. RAII throughout.

## Hard Constraints (non-negotiable)
- C++17 strictly. No C++20 features.
- unique_ptr for all ownership. No raw new/delete. No raw owning pointers in public API.
- MVC separation: CLIView (View), NoteManager (Controller), Note hierarchy (Model).
- Google Test required. cmake --build and ctest must both exit 0 before any slice is complete.
- SecureNote: std::vector<std::byte> for ciphertext only. No plaintext in any member, log, or file.
- EncryptionEngine and StorageInterface are pure abstract interfaces. No concrete crypto in public APIs.

## Build System
CMake. nlohmann/json as header-only dependency. OpenSSL libcrypto declared but not linked yet.

## Traceability Rule
Every class header and every implemented function must include:
// Traceability: <REQ-ID> | UML: <ClassName.methodName>

## Current Sprint
Sprint Zero + Note Creation slice (US-01 / FR-1) only.
Do NOT implement: FileStorage body, AESEngine body, SecureNote passphrase gate,
edit, delete, search, VoiceNote audio, or VersionHistory population.