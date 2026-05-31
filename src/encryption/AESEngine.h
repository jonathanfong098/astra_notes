#pragma once
// Traceability: SPR-1, SPR-2 | UML: AESEngine

#include "EncryptionEngine.h"
#include <vector>
#include <cstddef>
#include <string>

// Concrete AES-256-GCM implementation via OpenSSL libcrypto.
// Key derived from passphrase using PBKDF2-SHA256 (100,000 iterations).
//
// Wire format (output of encrypt):
//   SALT (16 bytes) | IV (12 bytes) | TAG (16 bytes) | CIPHERTEXT (N bytes)
//
// SPR-2: AESEngine is only ever accessed through EncryptionEngine& references.
// No production class (NoteManager, SecureNote, FileStorage) includes this header.
class AESEngine final : public EncryptionEngine {
public:
    // Encrypts plaintext with AES-256-GCM. Random salt and IV generated per call.
    // Returns SALT || IV || TAG || CIPHERTEXT as a single byte vector.
    // Throws std::runtime_error on OpenSSL failure.
    // Traceability: SPR-1, SPR-2 | UML: AESEngine.encrypt
    std::vector<std::byte> encrypt(const std::string& plaintext,
                                    const std::string& passphrase) override;

    // Extracts SALT, IV, TAG from the byte vector, re-derives the key,
    // decrypts, and verifies the authentication tag.
    // Throws std::runtime_error if the tag fails (wrong passphrase or tampering).
    // Traceability: SPR-1, SPR-2 | UML: AESEngine.decrypt
    std::string decrypt(const std::vector<std::byte>& ciphertext,
                        const std::string& passphrase) override;
};
