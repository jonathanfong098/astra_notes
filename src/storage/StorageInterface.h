#pragma once
// Traceability: SPR-2, FR-4 (refined) | UML: StorageInterface

#include <vector>
#include <memory>
#include <unordered_map>
#include "../model/Note.h"

class StorageInterface {
public:
    virtual ~StorageInterface() = default;

    // Persists a single note.
    // Traceability: FR-5 (refined stub) | UML: StorageInterface.saveNote
    virtual void saveNote(const Note& note) = 0;

    // Atomically persists the entire note collection.
    // Design decision (FR-4): saveAll() is preferred over a saveNote() loop because FR-4 requires
    // a single atomic write. FileStorage overrides this with a tmp-then-rename write.
    // Default implementation loops and calls saveNote() — correct for NullStorage.
    // Traceability: FR-4 (refined) | UML: StorageInterface.saveAll
    virtual void saveAll(const std::unordered_map<UUID, std::unique_ptr<Note>>& notes) {
        for (const auto& [uuid, note] : notes) saveNote(*note);
    }

    // Loads and reconstructs all persisted notes.
    // Traceability: FR-4 (refined) | UML: StorageInterface.loadNotes
    virtual std::vector<std::unique_ptr<Note>> loadNotes() = 0;
};
