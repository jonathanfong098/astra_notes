# AstraNotes — Library Decision Log

> **Purpose of this file:** NFR-4 requires that every third-party library adopted in AstraNotes
> be documented here before it is used. This file must be committed to version control.
> No library may be added to CMakeLists.txt without a corresponding entry below.

---

## Entry 1 — nlohmann/json v3.11.3

**Date adopted:** 2026-05-11 (Sprint Zero)
**Adopted by:** Jonathan Fong
**Status:** ✅ In use — linked to `astra_core` via FetchContent

### What the project could not achieve without it

AstraNotes requires serializing and deserializing a heterogeneous collection of Note subtypes
(TextNote, VoiceNote, SecureNote) to and from a local JSON file (FR-4). The C++17 standard
library provides no JSON parser or serializer. The hand-rolled alternative would require writing
a recursive-descent parser, a serializer, error handling for malformed input, and per-record
quarantine logic — all of which are correctness-critical and outside the scope of this project.
nlohmann/json provides a well-tested, header-only implementation of all of this.

### Tradeoffs versus a standard library alternative

There is no standard library JSON alternative in C++17. The closest alternatives considered:

| Option | Tradeoff |
|---|---|
| Hand-rolled JSON serializer | Feasible for a fixed schema but requires writing and testing a parser from scratch. Every edge case (malformed records, partial writes, UTF-8) must be handled manually. High risk of correctness gaps. |
| RapidJSON | SAX-style API requires significantly more boilerplate for AstraNotes' use case. Not header-only. More complex CMake integration. |
| nlohmann/json | Header-only. Single `#include`. Intuitive `[]` operator API. First-class C++17 support including `std::optional` and `std::variant`. Clean FetchContent integration. |

**Decision:** nlohmann/json is the lowest-friction choice that satisfies FR-4 without introducing
unnecessary complexity or a compiled dependency.

### C++17 compatibility evidence

- nlohmann/json v3.11.3 explicitly requires and supports C++17 (`CMAKE_CXX_STANDARD 17`).
- It is header-only — no compiled library, no ABI compatibility concerns.
- It uses `std::string_view`, `if constexpr`, and structured bindings — all C++17 features
  already required by AstraNotes.
- Confirmed: `cmake --build` and `ctest` both exit 0 with nlohmann/json fetched via FetchContent
  alongside Google Test v1.14.0. No linkage conflicts.

### GTest linkage test result

**Result: PASS**

nlohmann/json is header-only and introduces no compiled object files. It links to `astra_core`
as `nlohmann_json::nlohmann_json` (FetchContent interface target). Google Test links to
`astra_core` separately as `GTest::gtest_main`. No symbol conflicts. Both `cmake --build` and
`ctest --test-dir build --output-on-failure` exit 0 with both libraries present.

### License classification

**License:** MIT License

Permissive open-source license. No copyleft obligations. No restrictions on use in academic or
commercial projects. Attribution required in source or documentation if redistributed — not
applicable here as the library is fetched at build time and not redistributed as source.

**Acceptable for AstraNotes:** Yes, with no restrictions.

### Supply-chain risk assessment

| Factor | Assessment |
|---|---|
| Maintainership | Active. Maintained by Niels Lohmann with regular releases. v3.11.3 released 2023. |
| Adoption | One of the most widely used C++ JSON libraries. Used by hundreds of thousands of projects. |
| Source | Fetched directly from `https://github.com/nlohmann/json.git` at tag `v3.11.3` via `GIT_SHALLOW TRUE`. Pinned to an exact release tag — no floating branch. |
| Vulnerability history | No known CVEs affecting the subset of features used (parse, serialize, basic types). |
| Dependency chain | Zero runtime dependencies. Header-only. No transitive supply-chain risk. |
| Worst-case failure | If the GitHub repository became unavailable, the build would fail at configure time. Mitigation: vendor the header into the repo if this becomes a concern. |

**Risk level: LOW**

---

## Entry 2 — OpenSSL libcrypto (AES-256-GCM)

**Date adopted:** Pending — required before Step 7 (AESEngine implementation)
**Status:** ⚠️ Declared in CMakeLists.txt via `find_package(OpenSSL QUIET)` but NOT yet linked
to any target. This entry must be completed and committed before Step 7 begins.

### What the project could not achieve without it

AstraNotes requires AES-256-GCM encryption for SecureNote content (SPR-1, FR-5). AES-256-GCM
provides authenticated encryption — it simultaneously encrypts the content and produces an
authentication tag that detects tampering. The C++17 standard library provides no cryptographic
primitives. A hand-rolled AES implementation would be cryptographically dangerous and is
explicitly out of scope for a student project.

### Tradeoffs versus a standard library alternative

There is no standard library cryptography alternative in C++17.

| Option | Tradeoff |
|---|---|
| Hand-rolled AES | Cryptographically dangerous. Any implementation error creates a security vulnerability. Never acceptable for production or academic security work. |
| libsodium | Simpler API than OpenSSL for symmetric encryption. However, requires a separate installation and has less predictable availability across macOS/Linux/Windows without a package manager. |
| OpenSSL libcrypto | Available by default on macOS (via system OpenSSL or Homebrew) and Linux. Widely used. CMake has first-class `find_package(OpenSSL)` support. Well-documented EVP API for AES-256-GCM. |

**Decision:** OpenSSL libcrypto is the most portable and available option given the cross-platform
target (macOS, Linux, Windows) and the CMake build system already in use.

### C++17 compatibility evidence

- OpenSSL is a C library. It is compatible with any C++ standard. No C++17-specific concerns.
- The EVP API (`EVP_EncryptInit_ex`, `EVP_EncryptUpdate`, `EVP_EncryptFinal_ex`) used for
  AES-256-GCM is stable across OpenSSL 1.1.x and 3.x.
- CMake `find_package(OpenSSL)` provides `OpenSSL::Crypto` as a modern imported target.

**⚠️ To be verified at Step 7:** Confirm `cmake --build` and `ctest` both exit 0 with
`target_link_libraries(astra_core PUBLIC OpenSSL::Crypto)` added to CMakeLists.txt.
Record the result here before merging Step 7.

### GTest linkage test result

**Result: PENDING — verify at Step 7**

OpenSSL::Crypto must be confirmed to link without symbol conflicts alongside nlohmann/json and
Google Test. Update this entry with the result when Step 7 is executed.

### License classification

**License:** Apache License 2.0 (OpenSSL 3.x) / OpenSSL License + SSLeay License (OpenSSL 1.1.x)

Both are permissive open-source licenses acceptable for academic use. OpenSSL 3.x (Apache 2.0)
is simpler and preferred if available on the build system.

**Acceptable for AstraNotes:** Yes, with no restrictions for academic use.

### Supply-chain risk assessment

| Factor | Assessment |
|---|---|
| Maintainership | Actively maintained by the OpenSSL project. Security patches released regularly. |
| Adoption | Used by virtually every TLS-enabled application on the internet. |
| Source | System-installed library located via `find_package(OpenSSL QUIET)`. Not fetched from a remote source — uses the version installed on the build machine. |
| Vulnerability history | OpenSSL has had significant CVEs (Heartbleed, etc.) but these affect TLS, not the symmetric encryption (EVP AES-256-GCM) API used by AstraNotes. |
| Dependency chain | System library. No transitive CMake dependencies introduced. |
| Worst-case failure | If OpenSSL is not installed, `find_package` returns not-found and the build fails gracefully (AESEngine cannot be compiled). Mitigation: document installation instructions in README. |

**Risk level: LOW for the specific APIs used (EVP AES-256-GCM only)**

---

## Approval Record

| Library | Version | Approved By | Date | NFR-4 Satisfied |
|---|---|---|---|---|
| nlohmann/json | v3.11.3 | Jonathan Fong | 2026-05-11 | ✅ |
| OpenSSL libcrypto | system (1.1.x or 3.x) | Jonathan Fong | Pending Step 7 | ⚠️ Pending |