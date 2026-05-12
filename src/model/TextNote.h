#pragma once
// Traceability: FR-1 (refined) | UML: TextNote

#include "Note.h"
#include <string>

class TextNote : public Note {
public:
    TextNote(UUID uuid, std::string title, std::string body = "");

    // Traceability: FR-1 (refined) | UML: TextNote.getType
    std::string getType() const override;

    // Traceability: FR-1 (refined) | UML: TextNote.getBody
    const std::string& getBody() const;

    // Traceability: FR-1 (refined) | UML: TextNote.setBody
    void setBody(std::string body);

private:
    std::string body_;
};
