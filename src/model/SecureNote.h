#pragma once
// Traceability: FR-5 (refined), FR-5a (refined), SPR-1, SPR-2 | UML: SecureNote

#include "Note.h"
#include "../encryption/EncryptionEngine.h"
#include <vector>
#include <cstddef>
#include <string>
#include <variant>

// Traceability: FR-5a (refined) | UML: SecureNoteError
enum class SecureNoteError { WRONG_PASSPHRASE, INVALID_INPUT };

class SecureNote : public Note {
public:
    // EncryptionEngine is injected; SecureNote does NOT own it.
    // Traceability: SPR-2 | UML: SecureNote.SecureNote
    SecureNote(UUID uuid, std::string title, EncryptionEngine& engine);

    // Reconstruction constructor — restores ciphertext from persisted JSON record.
    // Traceability: FR-4 (refined) | UML: SecureNote.SecureNote
    SecureNote(UUID uuid, std::string title, std::vector<std::byte> ciphertext,
               EncryptionEngine& engine, std::time_t createdAt, std::time_t lastModifiedAt);

    // Traceability: FR-2 (refined stub) | UML: SecureNote.getType
    std::string getType() const override;

    // Returns ciphertext bytes. Empty until lock() is called.
    // Traceability: SPR-1 | UML: SecureNote.getCiphertext
    const std::vector<std::byte>& getCiphertext() const;

    // Returns mutable reference to ciphertext for zeroing on delete (FR-3, SPR-1).
    // Traceability: FR-3 (refined) | UML: SecureNote.getCiphertextMutable
    std::vector<std::byte>& getCiphertextMutable();

    // Encrypts plaintext via engine_ and stores result in ciphertext_.
    // Passphrase must be >= 8 chars; returns INVALID_INPUT otherwise.
    // Plaintext local copy is zeroed before returning (SPR-1).
    // Traceability: FR-5 (refined) | UML: SecureNote.lock
    Status lock(const std::string& plaintext, const std::string& passphrase);

    // Decrypts ciphertext_ via engine_ and returns plaintext to caller.
    // Passphrase must be >= 8 chars → INVALID_INPUT.
    // Wrong passphrase (engine throws) → WRONG_PASSPHRASE.
    // Decryption buffer is zeroed before returning whether success or failure (SPR-1).
    // Plaintext is a local variable only — never stored in any member (SPR-1).
    // Traceability: FR-5 (refined) | UML: SecureNote.unlock
    std::variant<std::string, SecureNoteError> unlock(const std::string& passphrase) const;

private:
    std::vector<std::byte> ciphertext_; // SPR-1: never std::string or char*
    EncryptionEngine& engine_;           // SPR-2: injected reference, not owned
};
