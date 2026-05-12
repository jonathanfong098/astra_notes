#pragma once
// Traceability: FR-1 (refined) | UML: NoteFactory

#include "../model/Note.h"
#include <memory>
#include <string>

class NoteFactory {
public:
    NoteFactory() = default;

    // Creates a new Note of the given type with a fresh UUID and the given title.
    // Supported types: "text", "voice". "secure" throws (requires EncryptionEngine).
    // Throws std::invalid_argument if title is empty or type is unrecognized.
    // Traceability: FR-1 (refined) | UML: NoteFactory.create
    std::unique_ptr<Note> create(const std::string& type,
                                 const std::string& title) const;

    // Reconstructs a Note from a persisted JSON record string.
    // STUB: always throws std::runtime_error("not yet implemented") this sprint.
    // Traceability: FR-5 (refined stub) | UML: NoteFactory.reconstructRecord
    std::unique_ptr<Note> reconstructRecord(const std::string& jsonRecord) const;
};
