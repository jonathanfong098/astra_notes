// Traceability: FR-4 (refined), FR-4a (refined), FR-4b (refined), SPR-1 | UML: FileStorage
#include "FileStorage.h"
#include "../model/TextNote.h"
#include "../model/VoiceNote.h"
#include "../model/SecureNote.h"

#include <nlohmann/json.hpp>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <cstdio>
#include <ctime>
#include <iostream>
#include <unordered_set>

namespace {

// Traceability: SPR-1 | encodes raw bytes as lowercase hex string (no plaintext)
std::string hexEncode(const std::vector<std::byte>& bytes) {
    std::ostringstream oss;
    oss << std::hex << std::setfill('0');
    for (auto b : bytes)
        oss << std::setw(2) << static_cast<int>(std::to_integer<uint8_t>(b));
    return oss.str();
}

// Serialises a single Note to a JSON object.
// Traceability: FR-4 (refined), SPR-1 | UML: FileStorage.saveAll
nlohmann::json noteToJson(const Note& note) {
    nlohmann::json j;
    j["uuid"]           = note.getUUID();
    j["type"]           = note.getType();
    j["title"]          = note.getTitle();
    j["createdAt"]      = note.getCreatedAt();
    j["lastModifiedAt"] = note.getLastModifiedAt();

    if (note.getType() == "text") {
        const auto* tn = dynamic_cast<const TextNote*>(&note);
        j["body"] = tn ? tn->getBody() : "";
    } else if (note.getType() == "voice") {
        const auto* vn = dynamic_cast<const VoiceNote*>(&note);
        j["audioPath"] = vn ? vn->getAudioPath() : "";
    } else if (note.getType() == "secure") {
        const auto* sn = dynamic_cast<const SecureNote*>(&note);
        j["ciphertext"] = sn ? hexEncode(sn->getCiphertext()) : "";
        // SPR-1: no "body" or "plaintext" field for SecureNote
    }
    return j;
}

} // anonymous namespace

// Traceability: FR-4 (refined) | UML: FileStorage.FileStorage
FileStorage::FileStorage(std::string filePath, NoteFactory& factory, EncryptionEngine* engine)
    : filePath_(std::move(filePath))
    , factory_(factory)
    , engine_(engine) {}

// Traceability: FR-4 (refined) | UML: FileStorage.saveNote
void FileStorage::saveNote(const Note&) {
    // No-op: use saveAll() for atomic write (see class doc).
}

// Traceability: FR-4 (refined) | UML: FileStorage.saveAll
void FileStorage::saveAll(const std::unordered_map<UUID, std::unique_ptr<Note>>& notes) {
    nlohmann::json arr = nlohmann::json::array();
    for (const auto& [uuid, note] : notes)
        arr.push_back(noteToJson(*note));

    const std::string tmpPath = filePath_ + ".tmp";
    {
        std::ofstream out(tmpPath);
        out << arr.dump(2);
    }
    // Atomic rename: if the write succeeded, replace the destination (FR-4).
    std::rename(tmpPath.c_str(), filePath_.c_str());
}

// Traceability: FR-4 (refined), FR-4a (refined), FR-4b (refined) | UML: FileStorage.loadNotes
std::vector<std::unique_ptr<Note>> FileStorage::loadNotes() {
    std::ifstream in(filePath_);
    if (!in.is_open()) return {}; // FR-4a: missing file is not an error

    nlohmann::json arr;
    try {
        in >> arr;
    } catch (const nlohmann::json::exception& e) {
        in.close(); // release file handle before rename (FR-4b)
        const std::string quarantinePath =
            filePath_ + ".quarantine." + std::to_string(std::time(nullptr));
        std::rename(filePath_.c_str(), quarantinePath.c_str());
        std::cerr << "FileStorage: corrupt file quarantined as "
                  << quarantinePath << ": " << e.what() << '\n';
        return {};
    }

    std::vector<std::unique_ptr<Note>> result;
    std::unordered_set<UUID> seen;

    for (const auto& record : arr) {
        try {
            const UUID uuid = record.at("uuid").get<std::string>();
            if (seen.count(uuid)) {
                // FR-4: UUID collision — discard second record
                std::cerr << "FileStorage: duplicate UUID discarded: " << uuid << '\n';
                continue;
            }
            auto note = factory_.reconstructRecord(record, engine_);
            if (!note) continue; // nullptr when SecureNote skipped (no engine)
            seen.insert(uuid);
            result.push_back(std::move(note));
        } catch (const std::exception& e) {
            std::cerr << "FileStorage: skipping malformed record: " << e.what() << '\n';
        }
    }
    return result;
}
