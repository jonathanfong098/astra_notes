#pragma once
// Traceability: FR-1a (refined), FR-2 (refined) | UML: NoteFactory.create, NoteManager.editTitle

#include <string>
#include <cctype>

namespace util {

// Trims leading and trailing ASCII whitespace from a string.
// Used by NoteFactory (title validation) and NoteManager (edit validation).
// Traceability: FR-1a (refined) | UML: NoteFactory.create
inline std::string trim(const std::string& s) {
    auto start = s.begin();
    while (start != s.end() && std::isspace(static_cast<unsigned char>(*start)))
        ++start;
    auto end = s.end();
    while (end != start && std::isspace(static_cast<unsigned char>(*(end - 1))))
        --end;
    return {start, end};
}

} // namespace util
