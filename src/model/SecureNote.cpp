// Traceability: SPR-1, SPR-2 | UML: SecureNote
#include "SecureNote.h"
#include <utility>

// Traceability: SPR-2 | UML: SecureNote.SecureNote
SecureNote::SecureNote(UUID uuid, std::string title, EncryptionEngine& engine)
    : Note(std::move(uuid), std::move(title))
    , engine_(engine) {}

// Traceability: FR-2 (refined stub) | UML: SecureNote.getType
std::string SecureNote::getType() const { return "secure"; }

// Traceability: SPR-1 | UML: SecureNote.getCiphertext
const std::vector<std::byte>& SecureNote::getCiphertext() const { return ciphertext_; }

// Traceability: FR-3 (refined) | UML: SecureNote.getCiphertextMutable
std::vector<std::byte>& SecureNote::getCiphertextMutable() { return ciphertext_; }
