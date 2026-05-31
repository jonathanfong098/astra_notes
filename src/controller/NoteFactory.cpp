// Traceability: FR-1 (refined) | UML: NoteFactory
#include "NoteFactory.h"
#include "../model/TextNote.h"
#include "../model/VoiceNote.h"

#include <random>
#include <sstream>
#include <iomanip>
#include <stdexcept>
#include <algorithm>
#include <cctype>

namespace {

// Generates a UUID v4-style string (8-4-4-4-12 hex), pseudo-random.
// Uses thread_local engine to avoid repeated seeding overhead.
std::string generateUUID() {
    static thread_local std::mt19937_64 rng{std::random_device{}()};
    std::uniform_int_distribution<uint64_t> dist;
    uint64_t hi = dist(rng);
    uint64_t lo = dist(rng);

    // Set version bits (4) and variant bits (10xx)
    hi = (hi & 0xFFFFFFFFFFFF0FFFULL) | 0x0000000000004000ULL;
    lo = (lo & 0x3FFFFFFFFFFFFFFFULL) | 0x8000000000000000ULL;

    std::ostringstream oss;
    oss << std::hex << std::setfill('0');
    oss << std::setw(8) << ((hi >> 32) & 0xFFFFFFFFULL) << '-';
    oss << std::setw(4) << ((hi >> 16) & 0xFFFFULL)     << '-';
    oss << std::setw(4) << (hi & 0xFFFFULL)              << '-';
    oss << std::setw(4) << ((lo >> 48) & 0xFFFFULL)      << '-';
    oss << std::setw(12) << (lo & 0x0000FFFFFFFFFFFFULL);
    return oss.str();
}

// Trims leading and trailing ASCII whitespace from a string.
std::string trim(const std::string& s) {
    auto start = s.begin();
    while (start != s.end() && std::isspace(static_cast<unsigned char>(*start)))
        ++start;
    auto end = s.end();
    while (end != start && std::isspace(static_cast<unsigned char>(*(end - 1))))
        --end;
    return {start, end};
}

} // anonymous namespace

// Traceability: FR-1 (refined) | UML: NoteFactory.create
std::unique_ptr<Note> NoteFactory::create(const std::string& type,
                                           const std::string& title) const {
    const std::string trimmedTitle = trim(title);
    if (trimmedTitle.empty())
        throw std::invalid_argument("Note title must not be empty");
    // Traceability: FR-1a (refined) | UML: NoteFactory.create
    if (trimmedTitle.size() > 255)
        throw std::invalid_argument("Note title must not exceed 255 characters");

    const UUID uuid = generateUUID();

    if (type == "text")
        return std::make_unique<TextNote>(uuid, trimmedTitle);

    if (type == "voice")
        return std::make_unique<VoiceNote>(uuid, trimmedTitle);

    if (type == "secure")
        throw std::runtime_error(
            "SecureNote requires an injected EncryptionEngine; "
            "use an overloaded create() — deferred to Sprint N");

    throw std::invalid_argument("Unrecognized note type: " + type);
}

// Traceability: FR-5 (refined stub) | UML: NoteFactory.reconstructRecord
std::unique_ptr<Note> NoteFactory::reconstructRecord(const std::string&) const {
    throw std::runtime_error("NoteFactory::reconstructRecord: not yet implemented");
}
