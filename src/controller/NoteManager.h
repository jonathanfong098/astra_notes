#pragma once
// Traceability: NFR-1, FR-1 (refined) | UML: NoteManager

#include "../model/Note.h"
#include "NoteFactory.h"
#include "../storage/StorageInterface.h"
#include <unordered_map>
#include <memory>
#include <string>
#include <vector>

class NoteManager {
public:
    // factory and storage are injected; NoteManager does NOT own them.
    // Traceability: NFR-1 | UML: NoteManager.NoteManager
    NoteManager(NoteFactory& factory, StorageInterface& storage);

    // Transfers ownership of note into notes_. Throws if UUID already exists.
    // Traceability: FR-1 (refined) | UML: NoteManager.add
    void add(std::unique_ptr<Note> note);

    // Removes the note with the given UUID; no-op if not found.
    // Traceability: FR-4 (refined stub) | UML: NoteManager.remove
    void remove(const UUID& uuid);

    // Returns a const pointer to the note, or nullptr if UUID not found.
    // Traceability: FR-1 (refined) | UML: NoteManager.findByUUID
    const Note* findByUUID(const UUID& uuid) const;

    // Returns titles of all notes whose title contains query (case-insensitive).
    // STUB: returns empty vector this sprint.
    // Traceability: FR-6 (refined stub) | UML: NoteManager.searchByTitle
    std::vector<std::string> searchByTitle(const std::string& query) const;

    // Persists all notes via storage_. STUB: no-op this sprint.
    // Traceability: FR-5 (refined stub) | UML: NoteManager.persistAll
    void persistAll() const;

    // Loads notes from storage_. STUB: no-op this sprint.
    // Traceability: FR-5 (refined stub) | UML: NoteManager.loadAll
    void loadAll();

    // Exposes the notes map for read-only iteration by CLIView.
    // Traceability: NFR-1 | UML: NoteManager.getNotes
    const std::unordered_map<UUID, std::unique_ptr<Note>>& getNotes() const;

private:
    // NFR-1: unordered_map gives O(1) average UUID lookup vs O(log n) for std::map.
    std::unordered_map<UUID, std::unique_ptr<Note>> notes_; // owned
    NoteFactory& factory_;                         // injected, not owned
    StorageInterface& storage_;                    // injected, not owned
};
