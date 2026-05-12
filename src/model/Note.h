#pragma once
// Traceability: FR-1 (refined), NFR-1 | UML: Note

#include <string>
#include <memory>
#include <ctime>
#include "VersionHistory.h"

// UUID is a std::string formatted as 8-4-4-4-12 hex (generated in NoteFactory).
using UUID = std::string;

class Note {
public:
    Note(UUID uuid, std::string title);
    virtual ~Note() = default;

    // Traceability: FR-1 (refined) | UML: Note.getUUID
    const UUID& getUUID() const;

    // Traceability: FR-1 (refined) | UML: Note.getTitle
    const std::string& getTitle() const;

    // Traceability: FR-1 (refined) | UML: Note.getCreatedAt
    std::time_t getCreatedAt() const;

    // Traceability: NFR-3 (refined stub) | UML: Note.getVersionHistory
    const VersionHistory& getVersionHistory() const;

    // Traceability: FR-1 (refined) | UML: Note.getType
    virtual std::string getType() const = 0;

protected:
    UUID uuid_;
    std::string title_;
    std::time_t createdAt_;
    std::unique_ptr<VersionHistory> history_;
};
