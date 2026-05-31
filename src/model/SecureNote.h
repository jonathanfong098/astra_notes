#pragma once
// Traceability: SPR-1, SPR-2 | UML: SecureNote

#include "Note.h"
#include "../encryption/EncryptionEngine.h"
#include <vector>
#include <cstddef>

class SecureNote : public Note {
public:
    // EncryptionEngine is injected; SecureNote does NOT own it.
    // Traceability: SPR-2 | UML: SecureNote.SecureNote
    SecureNote(UUID uuid, std::string title, EncryptionEngine& engine);

    // Traceability: FR-2 (refined stub) | UML: SecureNote.getType
    std::string getType() const override;

    // Returns ciphertext bytes. Always empty until encryption is implemented (Sprint N).
    // Traceability: SPR-1 | UML: SecureNote.getCiphertext
    const std::vector<std::byte>& getCiphertext() const;

    // Returns mutable reference to ciphertext for zeroing on delete (FR-3, SPR-1).
    // Traceability: FR-3 (refined) | UML: SecureNote.getCiphertextMutable
    std::vector<std::byte>& getCiphertextMutable();

    // TODO: Sprint N — implement encrypt(passphrase) and decrypt(passphrase)
    // using engine_. Do NOT implement this sprint (SPR-1, SPR-2 scope gate).

private:
    std::vector<std::byte> ciphertext_; // SPR-1: never std::string or char*
    EncryptionEngine& engine_;           // SPR-2: injected reference, not owned
};
