// Traceability: NFR-1, FR-1 (refined) | UML: NoteManager
#include "NoteManager.h"
#include <stdexcept>
#include <algorithm>
#include <cctype>

// Traceability: NFR-1 | UML: NoteManager.NoteManager
NoteManager::NoteManager(NoteFactory& factory, StorageInterface& storage)
    : factory_(factory), storage_(storage) {}

// Traceability: FR-1 (refined) | UML: NoteManager.add
void NoteManager::add(std::unique_ptr<Note> note) {
    if (!note) throw std::invalid_argument("NoteManager::add: null note");
    const UUID id = note->getUUID();
    if (notes_.count(id))
        throw std::invalid_argument("NoteManager::add: UUID collision: " + id);
    notes_.emplace(id, std::move(note));
}

// Traceability: FR-4 (refined stub) | UML: NoteManager.remove
void NoteManager::remove(const UUID& uuid) {
    notes_.erase(uuid);
}

// Traceability: FR-1 (refined) | UML: NoteManager.findByUUID
const Note* NoteManager::findByUUID(const UUID& uuid) const {
    auto it = notes_.find(uuid);
    return (it != notes_.end()) ? it->second.get() : nullptr;
}

// Traceability: FR-6 (refined) | UML: NoteManager.searchByTitle
std::vector<std::string> NoteManager::searchByTitle(const std::string& query) const {
    auto toLower = [](std::string s) {
        std::transform(s.begin(), s.end(), s.begin(),
            [](unsigned char c){ return std::tolower(c); });
        return s;
    };
    const std::string lowerQuery = toLower(query);
    std::vector<std::string> results;
    for (const auto& [uuid, note] : notes_) {
        if (toLower(note->getTitle()).find(lowerQuery) != std::string::npos)
            results.push_back(note->getTitle());
    }
    return results;
}

// Traceability: FR-5 (refined stub) | UML: NoteManager.persistAll
void NoteManager::persistAll() const {
    // stub — FileStorage body deferred
}

// Traceability: FR-5 (refined stub) | UML: NoteManager.loadAll
void NoteManager::loadAll() {
    // stub — FileStorage body deferred
}

// Traceability: NFR-1 | UML: NoteManager.getNotes
const std::map<UUID, std::unique_ptr<Note>>& NoteManager::getNotes() const {
    return notes_;
}
