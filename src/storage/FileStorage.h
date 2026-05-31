#pragma once
// Traceability: FR-4 (refined), FR-4a (refined), SPR-1 | UML: FileStorage

#include "StorageInterface.h"
#include "../controller/NoteFactory.h"
#include "../encryption/EncryptionEngine.h"
#include <string>

// Concrete StorageInterface that reads/writes notes.json via nlohmann::json.
//
// Design decision: saveAll() is implemented with an atomic tmp-then-rename write (FR-4).
// saveNote() is a no-op — callers must use saveAll() via NoteManager::persistAll().
//
// engine: optional. Required only to reconstruct SecureNote records on load.
// If nullptr, SecureNote records are silently skipped during loadNotes().
class FileStorage final : public StorageInterface {
public:
    // Traceability: FR-4 (refined) | UML: FileStorage.FileStorage
    explicit FileStorage(std::string filePath, NoteFactory& factory,
                         EncryptionEngine* engine = nullptr);

    // No-op: FileStorage always writes atomically via saveAll().
    // Traceability: FR-4 (refined) | UML: FileStorage.saveNote
    void saveNote(const Note& note) override;

    // Serialises all notes to filePath_.tmp then renames to filePath_ (FR-4 atomic write).
    // SecureNote ciphertext is hex-encoded (SPR-1: no plaintext on disk).
    // Traceability: FR-4 (refined) | UML: FileStorage.saveAll
    void saveAll(const std::unordered_map<UUID, std::unique_ptr<Note>>& notes) override;

    // Deserialises notes from filePath_. Missing file returns empty vector (FR-4a).
    // Uses NoteFactory::reconstructRecord() for each valid record.
    // UUID collisions: first record wins, second discarded (FR-4).
    // Traceability: FR-4 (refined), FR-4a (refined) | UML: FileStorage.loadNotes
    std::vector<std::unique_ptr<Note>> loadNotes() override;

private:
    std::string      filePath_;
    NoteFactory&     factory_;           // injected; not owned
    EncryptionEngine* engine_;           // injected; not owned; may be nullptr
};
