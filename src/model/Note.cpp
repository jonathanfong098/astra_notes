// Traceability: FR-1 (refined), NFR-1 | UML: Note
#include "Note.h"
#include <utility>
#include <stdexcept>

// Traceability: FR-1 (refined), FR-2 (refined) | UML: Note.Note
Note::Note(UUID uuid, std::string title)
    : uuid_(std::move(uuid))
    , title_(std::move(title))
    , createdAt_(std::time(nullptr))
    , lastModifiedAt_(createdAt_)
    , history_(std::make_unique<VersionHistory>())
{
    if (uuid_.empty()) throw std::invalid_argument("UUID must not be empty");
}

// Traceability: FR-4 (refined) | UML: Note.Note
Note::Note(UUID uuid, std::string title, std::time_t createdAt, std::time_t lastModifiedAt)
    : uuid_(std::move(uuid))
    , title_(std::move(title))
    , createdAt_(createdAt)
    , lastModifiedAt_(lastModifiedAt)
    , history_(std::make_unique<VersionHistory>())
{
    if (uuid_.empty()) throw std::invalid_argument("UUID must not be empty");
}

// Traceability: FR-1 (refined) | UML: Note.getUUID
const UUID& Note::getUUID() const { return uuid_; }

// Traceability: FR-1 (refined) | UML: Note.getTitle
const std::string& Note::getTitle() const { return title_; }

// Traceability: FR-1 (refined) | UML: Note.getCreatedAt
std::time_t Note::getCreatedAt() const { return createdAt_; }

// Traceability: FR-2 (refined) | UML: Note.getLastModifiedAt
std::time_t Note::getLastModifiedAt() const { return lastModifiedAt_; }

// Traceability: NFR-3 (refined) | UML: Note.getVersionHistory
const VersionHistory& Note::getVersionHistory() const { return *history_; }

// Traceability: NFR-3 (refined) | UML: Note.getVersionHistory
VersionHistory& Note::getVersionHistory() { return *history_; }

// Traceability: FR-2 (refined) | UML: Note.setTitle
void Note::setTitle(const std::string& title) { title_ = title; }

// Traceability: FR-2 (refined) | UML: Note.refreshLastModified
void Note::refreshLastModified() { lastModifiedAt_ = std::time(nullptr); }
