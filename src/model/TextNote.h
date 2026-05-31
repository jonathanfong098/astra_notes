#pragma once
// Traceability: FR-1 (refined) | UML: TextNote

#include "Note.h"
#include <string>

class TextNote : public Note {
public:
    TextNote(UUID uuid, std::string title, std::string body = "");

    // Reconstruction constructor — restores from persisted JSON record via NoteFactory::reconstructRecord.
    // Traceability: FR-4 (refined) | UML: TextNote.TextNote
    TextNote(UUID uuid, std::string title, std::string body,
             std::time_t createdAt, std::time_t lastModifiedAt);

    // Traceability: FR-1 (refined) | UML: TextNote.getType
    std::string getType() const override;

    // Traceability: FR-1 (refined) | UML: TextNote.getBody
    const std::string& getBody() const;

    // Traceability: FR-1 (refined) | UML: TextNote.setBody
    void setBody(std::string body);

private:
    std::string body_;
};
