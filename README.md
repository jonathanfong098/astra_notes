# AstraNotes

A C++17 CLI note-taking application built with MVC architecture. The Model layer comprises the `Note` hierarchy (`TextNote`, `VoiceNote`, `SecureNote`), the Controller layer is `NoteManager` and `NoteFactory`, and the View layer is `CLIView`. Storage and encryption are provided via pure abstract interfaces.

**Build:**
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Debug && cmake --build build && ctest --test-dir build --output-on-failure
```
