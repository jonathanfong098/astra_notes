#pragma once
// Traceability: FR-1 (refined), FR-4 (refined) | UML: NoteFactory

#include "../model/Note.h"
#include "../encryption/EncryptionEngine.h"
#include <nlohmann/json.hpp>
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

    // Reconstructs a Note from a persisted JSON record (FR-4).
    // engine: required for SecureNote; if nullptr and type is "secure", returns nullptr (record skipped).
    // Uses stored UUID and timestamps — does NOT generate a new UUID.
    // Traceability: FR-4 (refined) | UML: NoteFactory.reconstructRecord
    std::unique_ptr<Note> reconstructRecord(const nlohmann::json& record,
                                             EncryptionEngine* engine = nullptr) const;
};
