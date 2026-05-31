#pragma once
// Traceability: FR-1 (refined), FR-2, FR-3, NFR-1 | UML: Note

#include <string>
#include <memory>
#include <ctime>
#include "VersionHistory.h"

// UUID is a std::string formatted as 8-4-4-4-12 hex (generated in NoteFactory).
using UUID = std::string;

// Traceability: FR-2, FR-3, FR-5a | UML: Status
enum class Status { OK, NOT_FOUND, INVALID_INPUT };

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

    // Traceability: FR-2 (refined) | UML: Note.getLastModifiedAt
    std::time_t getLastModifiedAt() const;

    // Traceability: NFR-3 (refined stub) | UML: Note.getVersionHistory
    const VersionHistory& getVersionHistory() const;

    // Traceability: FR-1 (refined) | UML: Note.getType
    virtual std::string getType() const = 0;

    // Sets title_; caller (NoteManager) is responsible for validation before calling.
    // Traceability: FR-2 (refined) | UML: Note.setTitle
    void setTitle(const std::string& title);

    // Updates lastModifiedAt_ to the current time. Called by NoteManager on edit success.
    // Traceability: FR-2 (refined) | UML: Note.refreshLastModified
    void refreshLastModified();

protected:
    UUID uuid_;
    std::string title_;
    std::time_t createdAt_;
    std::time_t lastModifiedAt_;
    std::unique_ptr<VersionHistory> history_;
};
