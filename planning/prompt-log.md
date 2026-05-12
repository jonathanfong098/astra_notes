# AstraNotes Prompt Log

## Entry 1

- **Date:** 2026-05-11
- **Task:** Sprint Zero bootstrap (S0-1 through S0-10) + Note Creation slice (US-01 / FR-1)
- **Prompt summary:** Self-contained Claude Code prompt covering: repository scaffold, root and test CMakeLists.txt with Google Test v1.14.0 and nlohmann/json v3.11.3 via FetchContent, all interface headers (Note abstract base, TextNote/VoiceNote/SecureNote stubs, VersionHistory/VersionEntry stubs, StorageInterface pure abstract, EncryptionEngine pure abstract, NoteFactory stateless factory, NoteManager controller, CLIView boundary), NoteFactory.create implementation with pseudo-UUID generation, NoteManager.add + findByUUID implementation, three GTest cases (happy path, empty-title rejection, UUID uniqueness), main.cpp CLI loop with [n]/[l]/[s]/[q] menu, traceability comments throughout, DoD checklist, and lab-vs-UML reconciliation note.
- **Output summary:** All Sprint Zero tasks S0-1 through S0-10 completed. Note Creation slice (US-01 / FR-1) implemented with AC-01-1, AC-01-2, AC-01-3 verified by GTest. cmake --build and ctest both exit 0.
- **Decision:** Profile and Settings implemented as a single `[s] settings` CLI stub (prints "Settings: not yet implemented.") in main.cpp. No SettingsManager or ProfileService class created.
- **Rationale:** The Week 6 lab deliverable asks for a menu/profile/settings/notes workspace shell. The validated AstraNotes UML defines CLIView with only three methods (renderNoteList, renderNote, promptInput). Inventing new classes not in the UML would contradict the design contract and create untraceable divergence. The stub satisfies the lab requirement (a settings menu entry is present) while preserving MVC architecture integrity.
