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

    // Removes the note by UUID. Returns OK if deleted, NOT_FOUND if absent.
    // For SecureNote: zeroes ciphertext_ before deallocation (FR-3, SPR-1).
    // Traceability: FR-3 (refined) | UML: NoteManager.remove
    Status remove(const UUID& uuid);

    // Updates the title of any note type. Validates per FR-1a (non-empty, ≤255 chars).
    // Returns NOT_FOUND if UUID absent, INVALID_INPUT if title is invalid.
    // Atomic: note is unchanged if validation fails.
    // Traceability: FR-2 (refined) | UML: NoteManager.editTitle
    Status editTitle(const UUID& uuid, const std::string& newTitle);

    // Updates the body of a TextNote or the audioPath of a VoiceNote.
    // Returns NOT_FOUND if UUID absent, INVALID_INPUT if SecureNote.
    // Traceability: FR-2 (refined) | UML: NoteManager.editBody
    Status editBody(const UUID& uuid, const std::string& newBody);

    // Re-encrypts a SecureNote with newBody using the given passphrase.
    // Returns NOT_FOUND if UUID absent, INVALID_INPUT if not a SecureNote or passphrase < 8 chars.
    // Traceability: FR-5 (refined) | UML: NoteManager.relockSecureBody
    Status relockSecureBody(const UUID& uuid, const std::string& newBody, const std::string& passphrase);

    // Returns a const pointer to the note, or nullptr if UUID not found.
    // Traceability: FR-1 (refined) | UML: NoteManager.findByUUID
    const Note* findByUUID(const UUID& uuid) const;

    // Returns {UUID, title} pairs for all notes whose title contains query (case-insensitive).
    // Empty query returns all notes. No file I/O occurs.
    // Traceability: FR-6 (refined) | UML: NoteManager.searchByTitle
    std::vector<std::pair<UUID, std::string>> searchByTitle(const std::string& query) const;

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
