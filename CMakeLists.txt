cmake_minimum_required(VERSION 3.14)
project(AstraNotes CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

# nlohmann/json — header-only
FetchContent_Declare(
  nlohmann_json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG        v3.11.3
  GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(nlohmann_json)

# Google Test
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        v1.14.0
  GIT_SHALLOW    TRUE
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# OpenSSL — path hint for Homebrew on macOS; linked to astra_core in Step 7.
if(APPLE AND EXISTS /opt/homebrew/opt/openssl@3)
    list(PREPEND CMAKE_PREFIX_PATH /opt/homebrew/opt/openssl@3)
endif()
find_package(OpenSSL QUIET)

# Core library: all model, controller, and view sources
add_library(astra_core STATIC
    src/model/Note.cpp
    src/model/TextNote.cpp
    src/model/VoiceNote.cpp
    src/model/SecureNote.cpp
    src/model/VersionHistory.cpp
    src/controller/NoteFactory.cpp
    src/controller/NoteManager.cpp
    src/storage/FileStorage.cpp
    src/encryption/AESEngine.cpp
    src/view/CLIView.cpp
)
target_include_directories(astra_core PUBLIC
    src
)
target_link_libraries(astra_core PUBLIC nlohmann_json::nlohmann_json)
if(OPENSSL_FOUND)
    target_link_libraries(astra_core PUBLIC OpenSSL::Crypto)
endif()

# Main executable
add_executable(astra_notes src/main.cpp)
target_link_libraries(astra_notes PRIVATE astra_core)

# Tests
enable_testing()
add_subdirectory(test)
