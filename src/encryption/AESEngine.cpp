// Traceability: SPR-1, SPR-2 | UML: AESEngine
#include "AESEngine.h"

#include <openssl/evp.h>
#include <openssl/rand.h>

#include <stdexcept>
#include <memory>
#include <algorithm>
#include <cstdint>

namespace {

// Wire-format constants — must match between encrypt and decrypt.
constexpr int SALT_LEN    = 16; // PBKDF2 salt
constexpr int IV_LEN      = 12; // GCM recommended nonce length
constexpr int TAG_LEN     = 16; // GCM authentication tag
constexpr int KEY_LEN     = 32; // AES-256 key
constexpr int PBKDF2_ITER = 100000;

// Derives a 32-byte AES-256 key from passphrase + salt via PBKDF2-SHA256.
// Traceability: SPR-1 | UML: AESEngine.encrypt, AESEngine.decrypt
std::vector<uint8_t> deriveKey(const std::string& passphrase, const uint8_t* salt) {
    std::vector<uint8_t> key(KEY_LEN, 0);
    if (PKCS5_PBKDF2_HMAC(
            passphrase.data(), static_cast<int>(passphrase.size()),
            salt, SALT_LEN,
            PBKDF2_ITER,
            EVP_sha256(),
            KEY_LEN, key.data()) != 1) {
        throw std::runtime_error("AESEngine: PBKDF2 key derivation failed");
    }
    return key;
}

using CtxPtr = std::unique_ptr<EVP_CIPHER_CTX, decltype(&EVP_CIPHER_CTX_free)>;

CtxPtr makeCtx() {
    CtxPtr ctx(EVP_CIPHER_CTX_new(), EVP_CIPHER_CTX_free);
    if (!ctx) throw std::runtime_error("AESEngine: failed to create EVP context");
    return ctx;
}

} // anonymous namespace

// Traceability: SPR-1, SPR-2 | UML: AESEngine.encrypt
std::vector<std::byte> AESEngine::encrypt(const std::string& plaintext,
                                           const std::string& passphrase) {
    std::vector<uint8_t> salt(SALT_LEN);
    std::vector<uint8_t> iv(IV_LEN);
    if (RAND_bytes(salt.data(), SALT_LEN) != 1 ||
        RAND_bytes(iv.data(),   IV_LEN)   != 1)
        throw std::runtime_error("AESEngine: random byte generation failed");

    auto key = deriveKey(passphrase, salt.data());
    auto ctx = makeCtx();

    if (EVP_EncryptInit_ex(ctx.get(), EVP_aes_256_gcm(), nullptr, nullptr, nullptr) != 1 ||
        EVP_CIPHER_CTX_ctrl(ctx.get(), EVP_CTRL_GCM_SET_IVLEN, IV_LEN, nullptr) != 1 ||
        EVP_EncryptInit_ex(ctx.get(), nullptr, nullptr, key.data(), iv.data()) != 1)
        throw std::runtime_error("AESEngine: encrypt init failed");

    // GCM is a stream cipher — output length equals input length.
    std::vector<uint8_t> cipherBuf(plaintext.size() + static_cast<size_t>(EVP_MAX_BLOCK_LENGTH));
    int outLen = 0;
    if (EVP_EncryptUpdate(ctx.get(), cipherBuf.data(), &outLen,
                           reinterpret_cast<const uint8_t*>(plaintext.data()),
                           static_cast<int>(plaintext.size())) != 1)
        throw std::runtime_error("AESEngine: encrypt update failed");
    int totalLen = outLen;

    // GCM final outputs 0 bytes but must be called to finalise.
    if (EVP_EncryptFinal_ex(ctx.get(), cipherBuf.data() + totalLen, &outLen) != 1)
        throw std::runtime_error("AESEngine: encrypt final failed");
    totalLen += outLen;
    cipherBuf.resize(static_cast<size_t>(totalLen));

    std::vector<uint8_t> tag(TAG_LEN);
    if (EVP_CIPHER_CTX_ctrl(ctx.get(), EVP_CTRL_GCM_GET_TAG, TAG_LEN, tag.data()) != 1)
        throw std::runtime_error("AESEngine: tag extraction failed");

    // Build output: SALT | IV | TAG | CIPHERTEXT
    std::vector<std::byte> out;
    out.reserve(static_cast<size_t>(SALT_LEN + IV_LEN + TAG_LEN + totalLen));
    for (auto b : salt)     out.push_back(std::byte{b});
    for (auto b : iv)       out.push_back(std::byte{b});
    for (auto b : tag)      out.push_back(std::byte{b});
    for (auto b : cipherBuf) out.push_back(std::byte{b});

    // SPR-1: zero key material before it goes out of scope.
    std::fill(key.begin(), key.end(), uint8_t{0});
    return out;
}

// Traceability: SPR-1, SPR-2 | UML: AESEngine.decrypt
std::string AESEngine::decrypt(const std::vector<std::byte>& ciphertext,
                                const std::string& passphrase) {
    const size_t minLen = static_cast<size_t>(SALT_LEN + IV_LEN + TAG_LEN);
    if (ciphertext.size() < minLen)
        throw std::runtime_error("AESEngine: ciphertext too short to be valid");

    const auto* raw     = reinterpret_cast<const uint8_t*>(ciphertext.data());
    const uint8_t* salt  = raw;
    const uint8_t* iv    = raw + SALT_LEN;
    const uint8_t* tag   = raw + SALT_LEN + IV_LEN;
    const uint8_t* enc   = raw + SALT_LEN + IV_LEN + TAG_LEN;
    const size_t   encLen = ciphertext.size() - minLen;

    auto key = deriveKey(passphrase, salt);
    auto ctx = makeCtx();

    if (EVP_DecryptInit_ex(ctx.get(), EVP_aes_256_gcm(), nullptr, nullptr, nullptr) != 1 ||
        EVP_CIPHER_CTX_ctrl(ctx.get(), EVP_CTRL_GCM_SET_IVLEN, IV_LEN, nullptr) != 1 ||
        EVP_DecryptInit_ex(ctx.get(), nullptr, nullptr, key.data(), iv) != 1)
        throw std::runtime_error("AESEngine: decrypt init failed");

    std::vector<uint8_t> plainBuf(encLen + static_cast<size_t>(EVP_MAX_BLOCK_LENGTH));
    int outLen = 0;
    if (EVP_DecryptUpdate(ctx.get(), plainBuf.data(), &outLen,
                           enc, static_cast<int>(encLen)) != 1)
        throw std::runtime_error("AESEngine: decrypt update failed");
    int totalLen = outLen;

    // Set expected tag before finalising — EVP_CTRL_GCM_SET_TAG needs non-const ptr.
    if (EVP_CIPHER_CTX_ctrl(ctx.get(), EVP_CTRL_GCM_SET_TAG, TAG_LEN,
                              const_cast<uint8_t*>(tag)) != 1)
        throw std::runtime_error("AESEngine: tag set failed");

    // Returns 1 on success, 0 on authentication failure.
    if (EVP_DecryptFinal_ex(ctx.get(), plainBuf.data() + totalLen, &outLen) != 1) {
        std::fill(key.begin(), key.end(), uint8_t{0});
        std::fill(plainBuf.begin(), plainBuf.end(), uint8_t{0});
        throw std::runtime_error("AESEngine: authentication failed (wrong passphrase or tampered data)");
    }
    totalLen += outLen;

    std::string result(reinterpret_cast<const char*>(plainBuf.data()),
                       static_cast<size_t>(totalLen));

    // SPR-1: zero sensitive buffers before they go out of scope.
    std::fill(key.begin(), key.end(), uint8_t{0});
    std::fill(plainBuf.begin(), plainBuf.end(), uint8_t{0});
    return result;
}
