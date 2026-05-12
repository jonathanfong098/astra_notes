# AstraNotes — Definition of Done Checklist
## Sprint Zero + Note Creation Slice (US-01 / FR-1) | 2026-05-11

### Header-File DoD
- [ ] All public methods in every header carry a one-line documentation comment.
- [ ] Ownership semantics are explicit in every class comment (unique_ptr, injected ref, etc.).
- [ ] No raw owning pointer appears in any public API.
- [ ] Every header file has `#pragma once`.
- [ ] Every header file has a traceability comment immediately after `#pragma once`.

### Build & Test DoD
- [ ] `cmake --build build` exits 0 with no warnings treated as errors.
- [ ] `ctest --test-dir build --output-on-failure` exits 0.
- [ ] At least 3 GTest cases pass (HappyPath, EmptyTitle, UUIDUniqueness).

### Implementation DoD
- [ ] Every implemented function has a traceability comment.
- [ ] No `new` or `delete` appears anywhere in the codebase.
- [ ] `SecureNote::ciphertext_` is `std::vector<std::byte>` (verified by grep).
- [ ] `StorageInterface` has no non-virtual members (verified by inspection).
- [ ] `EncryptionEngine` has no non-virtual members (verified by inspection).

### AI-Generated Code DoD
- [ ] Hallucination scan: every class name in code appears in the UML or is a stdlib/GTest type.
- [ ] Memory safety: no raw owning pointers, no manual `delete`, no use-after-`std::move`.
- [ ] Every non-trivial decision can be traced to a requirement ID in this prompt.
- [ ] `planning/prompt-log.md` has at least one entry for the 2026-05-11 session.
- [ ] `docs/ai-reflection.md` has at least one entry for the 2026-05-11 session.
- [ ] `docs/dod-checklist.md` is fully checked (all boxes ticked).

### Out-of-Scope Verification
- [ ] `FileStorage.cpp` does not exist.
- [ ] `AESEngine.cpp` does not exist.
- [ ] No passphrase gate exists in `SecureNote`.
- [ ] No edit or delete note behavior is implemented.
- [ ] `NoteManager::searchByTitle` returns `{}` (stub only).
- [ ] No audio handling exists in `VoiceNote`.
- [ ] `VersionHistory::entries_` is always empty (no `addEntry` or `push_back` call).
- [ ] No C++20 features appear anywhere.
