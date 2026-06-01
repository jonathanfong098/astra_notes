# AstraNotes

A C++17 CLI note-taking application built with MVC architecture. The Model layer comprises the `Note` hierarchy (`TextNote`, `VoiceNote`, `SecureNote`), the Controller layer is `NoteManager` and `NoteFactory`, and the View layer is `CLIView`. Storage and encryption are provided via pure abstract interfaces.

**Build:**
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Debug && cmake --build build && ctest --test-dir build --output-on-failure
```

**Run:**
```bash
./build/astra_notes
```
Notes are stored at `~/.astranotes/notes.json`.

**Known limitations (v1.0):**

> **SPR-3:** Concurrent access to the same storage file by two application instances is undefined behavior in v1.0. Only one instance of AstraNotes should run against a given storage file at a time. File locking is not implemented.
