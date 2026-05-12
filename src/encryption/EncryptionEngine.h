#pragma once
// Traceability: SPR-2 | UML: EncryptionEngine

#include <vector>
#include <cstddef>
#include <string>

class EncryptionEngine {
public:
    virtual ~EncryptionEngine() = default;

    // Traceability: SPR-2 | UML: EncryptionEngine.encrypt
    virtual std::vector<std::byte> encrypt(const std::string& plaintext,
                                            const std::string& passphrase) = 0;

    // Traceability: SPR-2 | UML: EncryptionEngine.decrypt
    virtual std::string decrypt(const std::vector<std::byte>& ciphertext,
                                const std::string& passphrase) = 0;
};
