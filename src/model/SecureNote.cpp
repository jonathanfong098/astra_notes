// Traceability: FR-5 (refined), FR-5a (refined), SPR-1, SPR-2 | UML: SecureNote
#include "SecureNote.h"
#include <utility>
#include <algorithm>

// Traceability: SPR-2 | UML: SecureNote.SecureNote
SecureNote::SecureNote(UUID uuid, std::string title, EncryptionEngine& engine)
    : Note(std::move(uuid), std::move(title))
    , engine_(engine) {}

// Traceability: FR-4 (refined) | UML: SecureNote.SecureNote
SecureNote::SecureNote(UUID uuid, std::string title, std::vector<std::byte> ciphertext,
                       EncryptionEngine& engine, std::time_t createdAt, std::time_t lastModifiedAt)
    : Note(std::move(uuid), std::move(title), createdAt, lastModifiedAt)
    , ciphertext_(std::move(ciphertext))
    , engine_(engine) {}

// Traceability: FR-2 (refined stub) | UML: SecureNote.getType
std::string SecureNote::getType() const { return "secure"; }

// Traceability: SPR-1 | UML: SecureNote.getCiphertext
const std::vector<std::byte>& SecureNote::getCiphertext() const { return ciphertext_; }

// Traceability: FR-3 (refined) | UML: SecureNote.getCiphertextMutable
std::vector<std::byte>& SecureNote::getCiphertextMutable() { return ciphertext_; }

// Traceability: FR-5 (refined) | UML: SecureNote.lock
Status SecureNote::lock(const std::string& plaintext, const std::string& passphrase) {
    // FR-5: passphrase must be at least 8 characters (checked via .size(), not strlen()).
    if (passphrase.size() < 8) return Status::INVALID_INPUT;

    // Make a local copy so we can zero it after encryption (SPR-1).
    std::string localPlaintext = plaintext;
    try {
        ciphertext_ = engine_.encrypt(localPlaintext, passphrase);
    } catch (...) {
        std::fill(localPlaintext.begin(), localPlaintext.end(), '\0');
        return Status::INVALID_INPUT;
    }
    // SPR-1: zero local plaintext copy — plaintext must not linger on the stack.
    std::fill(localPlaintext.begin(), localPlaintext.end(), '\0');
    return Status::OK;
}

// Traceability: FR-5 (refined) | UML: SecureNote.unlock
std::variant<std::string, SecureNoteError> SecureNote::unlock(const std::string& passphrase) {
    // FR-5a: reject empty or short passphrase before calling engine (INVALID_INPUT).
    if (passphrase.size() < 8) return SecureNoteError::INVALID_INPUT;

    std::string decryptBuf;
    try {
        decryptBuf = engine_.decrypt(ciphertext_, passphrase);
    } catch (...) {
        // SPR-1: zero whatever partial data the engine may have written.
        std::fill(decryptBuf.begin(), decryptBuf.end(), '\0');
        return SecureNoteError::WRONG_PASSPHRASE;
    }

    // Copy plaintext into the return value, then zero the buffer (SPR-1).
    // Plaintext lives only in the returned variant — never in any member.
    std::string result = decryptBuf;
    std::fill(decryptBuf.begin(), decryptBuf.end(), '\0');
    return result;
}
