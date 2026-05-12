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
