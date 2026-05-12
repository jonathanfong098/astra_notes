#pragma once
// Traceability: SPR-2 | UML: StorageInterface

#include <vector>
#include <memory>
#include "../model/Note.h"

class StorageInterface {
public:
    virtual ~StorageInterface() = default;

    // Persists a single note. STUB — body deferred to FileStorage in a later sprint.
    // Traceability: FR-5 (refined stub) | UML: StorageInterface.saveNote
    virtual void saveNote(const Note& note) = 0;

    // Loads and reconstructs all persisted notes. STUB — deferred to FileStorage.
    // Traceability: FR-5 (refined stub) | UML: StorageInterface.loadNotes
    virtual std::vector<std::unique_ptr<Note>> loadNotes() = 0;
};
