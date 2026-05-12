// Traceability: FR-1 (refined) | UML: TextNote
#include "TextNote.h"
#include <utility>

// Traceability: FR-1 (refined) | UML: TextNote.TextNote
TextNote::TextNote(UUID uuid, std::string title, std::string body)
    : Note(std::move(uuid), std::move(title))
    , body_(std::move(body)) {}

// Traceability: FR-1 (refined) | UML: TextNote.getType
std::string TextNote::getType() const { return "text"; }

// Traceability: FR-1 (refined) | UML: TextNote.getBody
const std::string& TextNote::getBody() const { return body_; }

// Traceability: FR-1 (refined) | UML: TextNote.setBody
void TextNote::setBody(std::string body) { body_ = std::move(body); }
