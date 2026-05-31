// Traceability: NFR-1, FR-1 (refined), FR-2 (refined), FR-3 (refined) | UML: NoteManager
#include "NoteManager.h"
#include "../model/TextNote.h"
#include "../model/SecureNote.h"
#include <stdexcept>
#include <algorithm>
#include <cctype>

namespace {

// Trims leading and trailing ASCII whitespace.
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

// Traceability: FR-3 (refined) | UML: NoteManager.remove
Status NoteManager::remove(const UUID& uuid) {
    auto it = notes_.find(uuid);
    if (it == notes_.end()) return Status::NOT_FOUND;

    // Zero ciphertext before deallocation for SecureNote (FR-3, SPR-1).
    if (auto* sn = dynamic_cast<SecureNote*>(it->second.get())) {
        auto& bytes = sn->getCiphertextMutable();
        std::fill(bytes.begin(), bytes.end(), std::byte{0});
    }
    notes_.erase(it);
    return Status::OK;
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

// Traceability: FR-2 (refined) | UML: NoteManager.editTitle
Status NoteManager::editTitle(const UUID& uuid, const std::string& newTitle) {
    auto it = notes_.find(uuid);
    if (it == notes_.end()) return Status::NOT_FOUND;

    auto* textNote = dynamic_cast<TextNote*>(it->second.get());
    if (!textNote) return Status::INVALID_INPUT;

    // Validate before any mutation (atomic semantics — FR-2).
    const std::string trimmed = trim(newTitle);
    if (trimmed.empty() || trimmed.size() > 255)
        return Status::INVALID_INPUT;

    textNote->setTitle(trimmed);
    textNote->refreshLastModified();
    return Status::OK;
}

// Traceability: FR-2 (refined) | UML: NoteManager.editBody
Status NoteManager::editBody(const UUID& uuid, const std::string& newBody) {
    auto it = notes_.find(uuid);
    if (it == notes_.end()) return Status::NOT_FOUND;

    auto* textNote = dynamic_cast<TextNote*>(it->second.get());
    if (!textNote) return Status::INVALID_INPUT;

    textNote->setBody(newBody);
    textNote->refreshLastModified();
    return Status::OK;
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
const std::unordered_map<UUID, std::unique_ptr<Note>>& NoteManager::getNotes() const {
    return notes_;
}
