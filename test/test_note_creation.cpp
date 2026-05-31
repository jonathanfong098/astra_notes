// Traceability: FR-1 (refined), FR-2 (refined), FR-3 (refined), FR-4 (refined) | UML: NoteFactory.create, NoteManager
#include <gtest/gtest.h>
#include "controller/NoteFactory.h"
#include "controller/NoteManager.h"
#include "storage/StorageInterface.h"
#include "storage/FileStorage.h"
#include "model/Note.h"
#include "model/TextNote.h"
#include "model/SecureNote.h"
#include "encryption/EncryptionEngine.h"
#include "encryption/AESEngine.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <variant>
#include <filesystem>
#include <fstream>

// Minimal no-op StorageInterface for unit tests — no file I/O.
class NullStorage final : public StorageInterface {
public:
    void saveNote(const Note&) override {}
    std::vector<std::unique_ptr<Note>> loadNotes() override { return {}; }
};

// Inline MockEncryptionEngine for SecureNote tests (SPR-2: no real crypto in unit tests).
// Traceability: SPR-2 | UML: EncryptionEngine
class MockEncryptionEngine : public EncryptionEngine {
public:
    std::vector<std::byte> encrypt(const std::string&, const std::string&) override {
        return {std::byte{0xDE}, std::byte{0xAD}};
    }
    std::string decrypt(const std::vector<std::byte>&, const std::string& pass) override {
        if (pass == correctPass_) return plaintext_;
        throw std::runtime_error("wrong passphrase");
    }
    std::string correctPass_ = "testpass1";
    std::string plaintext_   = "secret content";
};

// Fails the test if any storage method is called — used by D-series search tests
// to prove searchByTitle() performs no file I/O (FR-6 / US-06 AC-3).
class FailOnCallStorage final : public StorageInterface {
public:
    void saveNote(const Note&) override { FAIL() << "Search must not call saveNote"; }
    std::vector<std::unique_ptr<Note>> loadNotes() override {
        ADD_FAILURE() << "Search must not call loadNotes";
        return {};
    }
};

// Traceability: FR-1 (refined) | UML: NoteFactory.create, NoteManager.add
TEST(NoteCreation, HappyPath_TextNoteCreatedAndStored) {
    NoteFactory factory;
    NullStorage storage;
    NoteManager manager(factory, storage);

    auto note = factory.create("text", "Buy groceries");
    ASSERT_NE(note, nullptr);
    EXPECT_EQ(note->getType(), "text");
    EXPECT_EQ(note->getTitle(), "Buy groceries");
    EXPECT_FALSE(note->getUUID().empty());

    const UUID uuid = note->getUUID();
    manager.add(std::move(note));

    const Note* found = manager.findByUUID(uuid);
    ASSERT_NE(found, nullptr);
    EXPECT_EQ(found->getTitle(), "Buy groceries");
    EXPECT_EQ(found->getType(), "text");
}

// Traceability: FR-1 (refined) | UML: NoteFactory.create
TEST(NoteCreation, EmptyTitle_ThrowsInvalidArgument) {
    NoteFactory factory;
    EXPECT_THROW(factory.create("text", ""), std::invalid_argument);
    EXPECT_THROW(factory.create("text", "   "), std::invalid_argument);
}

// Traceability: FR-1 (refined) AC-01-3 | UML: NoteFactory.create
TEST(NoteCreation, TwoNotesWithSameTitle_ReceiveDistinctUUIDs) {
    NoteFactory factory;
    auto n1 = factory.create("text", "Idea");
    auto n2 = factory.create("text", "Idea");
    EXPECT_NE(n1->getUUID(), n2->getUUID());
}

// Traceability: FR-6 (refined) | UML: NoteManager.searchByTitle
TEST(SearchByTitle, CaseInsensitive_UpperQueryMatchesLowerTitle) {
    NoteFactory factory;
    NullStorage storage;
    NoteManager manager(factory, storage);

    manager.add(factory.create("text", "alpha notes"));
    manager.add(factory.create("text", "Beta reminder"));

    const auto results = manager.searchByTitle("ALPHA");
    ASSERT_EQ(results.size(), 1u);
    EXPECT_EQ(results[0], "alpha notes");
}

// Traceability: FR-6 (refined) | UML: NoteManager.searchByTitle
TEST(SearchByTitle, NoMatch_ReturnsEmptyVector) {
    NoteFactory factory;
    NullStorage storage;
    NoteManager manager(factory, storage);

    manager.add(factory.create("text", "Meeting notes"));

    EXPECT_TRUE(manager.searchByTitle("zzz").empty());
}

// Traceability: FR-6 (refined) | UML: NoteManager.searchByTitle
TEST(SearchByTitle, EmptyQuery_ReturnsAllNotes) {
    NoteFactory factory;
    FailOnCallStorage storage;
    NoteManager manager(factory, storage);

    manager.add(factory.create("text", "Alpha"));
    manager.add(factory.create("text", "Beta"));
    manager.add(factory.create("text", "Gamma"));

    const auto results = manager.searchByTitle("");
    EXPECT_EQ(results.size(), 3u);
}

// Traceability: FR-1a (refined) | UML: NoteFactory.create
TEST(NoteCreation, RejectsTitleExceeding255Chars) {
    NoteFactory factory;
    const std::string longTitle(256, 'x');
    EXPECT_THROW(factory.create("text", longTitle), std::invalid_argument);
}

// Traceability: FR-2 (refined) | UML: NoteManager.editBody
TEST(EditTextNote, UpdatesBodyAndTimestamp) {
    NoteFactory factory;
    NullStorage storage;
    NoteManager manager(factory, storage);

    auto note = factory.create("text", "My note");
    const UUID uuid = note->getUUID();
    const std::time_t originalCreatedAt = note->getCreatedAt();
    manager.add(std::move(note));

    const Status s = manager.editBody(uuid, "Updated body");
    EXPECT_EQ(s, Status::OK);

    const Note* updated = manager.findByUUID(uuid);
    ASSERT_NE(updated, nullptr);
    const auto* textNote = dynamic_cast<const TextNote*>(updated);
    ASSERT_NE(textNote, nullptr);
    EXPECT_EQ(textNote->getBody(), "Updated body");
    EXPECT_EQ(updated->getCreatedAt(), originalCreatedAt);
    EXPECT_GE(updated->getLastModifiedAt(), originalCreatedAt);
}

// Traceability: FR-2 (refined), FR-1a (refined) | UML: NoteManager.editTitle
TEST(EditTextNote, RejectsEmptyTitle) {
    NoteFactory factory;
    NullStorage storage;
    NoteManager manager(factory, storage);

    auto note = factory.create("text", "Original title");
    const UUID uuid = note->getUUID();
    manager.add(std::move(note));

    const Status s = manager.editTitle(uuid, "");
    EXPECT_EQ(s, Status::INVALID_INPUT);

    // Note must be fully unchanged after failed edit (atomic semantics — FR-2).
    const Note* unchanged = manager.findByUUID(uuid);
    ASSERT_NE(unchanged, nullptr);
    EXPECT_EQ(unchanged->getTitle(), "Original title");
}

// Traceability: FR-2 (refined) | UML: NoteManager.editTitle
TEST(EditTextNote, MissingUUID_ReturnsNotFound) {
    NoteFactory factory;
    NullStorage storage;
    NoteManager manager(factory, storage);

    const Status s = manager.editTitle("nonexistent-uuid", "New title");
    EXPECT_EQ(s, Status::NOT_FOUND);
}

// Traceability: FR-3 (refined) | UML: NoteManager.remove
TEST(DeleteNote, RemovesNoteFromCollection) {
    NoteFactory factory;
    NullStorage storage;
    NoteManager manager(factory, storage);

    auto note = factory.create("text", "To delete");
    const UUID uuid = note->getUUID();
    manager.add(std::move(note));

    const Status s = manager.remove(uuid);
    EXPECT_EQ(s, Status::OK);
    EXPECT_EQ(manager.findByUUID(uuid), nullptr);
}

// Traceability: FR-3 (refined), NFR-2 | UML: NoteManager.remove
TEST(DeleteNote, MissingUUID_ReturnsNotFound) {
    NoteFactory factory;
    NullStorage storage;
    NoteManager manager(factory, storage);

    const Status s = manager.remove("nonexistent-uuid");
    EXPECT_EQ(s, Status::NOT_FOUND);
}

// Traceability: FR-3 (refined) | UML: NoteManager.remove
TEST(DeleteNote, OtherNotesUntouched) {
    NoteFactory factory;
    NullStorage storage;
    NoteManager manager(factory, storage);

    auto n1 = factory.create("text", "Keep me 1");
    auto n2 = factory.create("text", "Delete me");
    auto n3 = factory.create("text", "Keep me 2");
    const UUID id1 = n1->getUUID();
    const UUID id2 = n2->getUUID();
    const UUID id3 = n3->getUUID();
    manager.add(std::move(n1));
    manager.add(std::move(n2));
    manager.add(std::move(n3));

    manager.remove(id2);

    EXPECT_NE(manager.findByUUID(id1), nullptr);
    EXPECT_EQ(manager.findByUUID(id2), nullptr);
    EXPECT_NE(manager.findByUUID(id3), nullptr);
}

// Traceability: FR-3 (refined), SPR-1 | UML: NoteManager.remove, SecureNote.getCiphertextMutable
TEST(DeleteNote, SecureNoteCiphertextZeroed) {
    MockEncryptionEngine engine;
    NoteFactory factory;
    NullStorage storage;
    NoteManager manager(factory, storage);

    // Construct SecureNote directly — NoteFactory::create("secure") is deferred to Sprint N.
    const UUID uuid = "secure-zeroing-test-uuid";
    auto secureNote = std::make_unique<SecureNote>(uuid, "Secret note", engine);

    // Populate ciphertext with non-zero bytes via getCiphertextMutable().
    auto& bytes = secureNote->getCiphertextMutable();
    bytes = {std::byte{0xDE}, std::byte{0xAD}, std::byte{0xBE}, std::byte{0xEF}};

    manager.add(std::move(secureNote));

    // Verify ciphertext is non-empty before delete.
    const auto* before = dynamic_cast<const SecureNote*>(manager.findByUUID(uuid));
    ASSERT_NE(before, nullptr);
    ASSERT_FALSE(before->getCiphertext().empty());

    // Delete — remove() must zero ciphertext_ before erase (FR-3, SPR-1).
    const Status s = manager.remove(uuid);
    EXPECT_EQ(s, Status::OK);
    EXPECT_EQ(manager.findByUUID(uuid), nullptr);
}

// ---------------------------------------------------------------------------
// Feature C — JSON Persistence (FR-4, FR-4a)
// All C-series tests use a per-test temp file; TearDown cleans it up.
// Traceability: FR-4 (refined), FR-4a (refined) | UML: FileStorage
// ---------------------------------------------------------------------------

class FileStorageTest : public ::testing::Test {
protected:
    std::string tmpPath_;
    void SetUp() override {
        tmpPath_ = (std::filesystem::temp_directory_path() / "astranotes_test.json").string();
    }
    void TearDown() override {
        std::filesystem::remove(tmpPath_);
        std::filesystem::remove(tmpPath_ + ".tmp");
        // Remove any quarantine files left by C4 (FR-4b)
        const auto parent = std::filesystem::path(tmpPath_).parent_path();
        const auto stem   = std::filesystem::path(tmpPath_).filename().string();
        for (const auto& entry : std::filesystem::directory_iterator(parent)) {
            const std::string name = entry.path().filename().string();
            if (name.rfind(stem + ".quarantine.", 0) == 0)
                std::filesystem::remove(entry.path());
        }
    }
};

// Traceability: FR-4 (refined) | UML: FileStorage.saveAll, FileStorage.loadNotes
TEST_F(FileStorageTest, RoundTrip_NotePreservesAllFields) {
    NoteFactory factory;
    FileStorage storage(tmpPath_, factory);
    NullStorage nullStorage;
    NoteManager manager(factory, nullStorage);

    manager.add(factory.create("text", "Round trip note"));
    const UUID uuid       = manager.getNotes().begin()->first;
    manager.editBody(uuid, "Persisted body");
    const std::time_t created  = manager.findByUUID(uuid)->getCreatedAt();
    const std::time_t modified = manager.findByUUID(uuid)->getLastModifiedAt();

    storage.saveAll(manager.getNotes());

    NoteManager manager2(factory, nullStorage);
    FileStorage storage2(tmpPath_, factory);
    auto loaded = storage2.loadNotes();
    ASSERT_EQ(loaded.size(), 1u);

    const Note* note = loaded[0].get();
    ASSERT_NE(note, nullptr);
    EXPECT_EQ(note->getUUID(),           uuid);
    EXPECT_EQ(note->getTitle(),          "Round trip note");
    EXPECT_EQ(note->getType(),           "text");
    EXPECT_EQ(note->getCreatedAt(),      created);
    EXPECT_EQ(note->getLastModifiedAt(), modified);

    const auto* tn = dynamic_cast<const TextNote*>(note);
    ASSERT_NE(tn, nullptr);
    EXPECT_EQ(tn->getBody(), "Persisted body");
}

// Traceability: FR-4 (refined) | UML: FileStorage.loadNotes, NoteFactory.reconstructRecord
TEST_F(FileStorageTest, LoadsAllThreeConcreteTypes) {
    MockEncryptionEngine engine;
    NoteFactory factory;
    NullStorage nullStorage;
    NoteManager manager(factory, nullStorage);

    manager.add(factory.create("text",  "Text note"));
    manager.add(factory.create("voice", "Voice note"));
    manager.add(std::make_unique<SecureNote>("secure-c2-uuid", "Secure note", engine));

    FileStorage storage(tmpPath_, factory, &engine);
    storage.saveAll(manager.getNotes());

    auto loaded = storage.loadNotes();
    ASSERT_EQ(loaded.size(), 3u);

    int textCount = 0, voiceCount = 0, secureCount = 0;
    for (const auto& n : loaded) {
        if      (n->getType() == "text")   ++textCount;
        else if (n->getType() == "voice")  ++voiceCount;
        else if (n->getType() == "secure") ++secureCount;
    }
    EXPECT_EQ(textCount,   1);
    EXPECT_EQ(voiceCount,  1);
    EXPECT_EQ(secureCount, 1);
}

// Traceability: FR-4a (refined) | UML: FileStorage.loadNotes
TEST_F(FileStorageTest, MissingFile_ReturnsEmptyCollection) {
    NoteFactory factory;
    FileStorage storage("/nonexistent/path/astranotes_missing.json", factory);
    auto notes = storage.loadNotes();
    EXPECT_TRUE(notes.empty());
}

// Traceability: FR-4b (refined) | UML: FileStorage.loadNotes
TEST_F(FileStorageTest, CorruptFile_QuarantinesAndReturnsEmpty) {
    // Write unparseable content to the file.
    { std::ofstream out(tmpPath_); out << "not valid json {{{"; }

    NoteFactory factory;
    FileStorage storage(tmpPath_, factory);
    auto notes = storage.loadNotes();
    EXPECT_TRUE(notes.empty());

    // Original file must be gone (renamed to quarantine).
    EXPECT_FALSE(std::filesystem::exists(tmpPath_));

    // A quarantine file with a Unix timestamp suffix must exist.
    bool quarantineFound = false;
    const auto parent = std::filesystem::path(tmpPath_).parent_path();
    const auto stem   = std::filesystem::path(tmpPath_).filename().string();
    for (const auto& entry : std::filesystem::directory_iterator(parent)) {
        if (entry.path().filename().string().rfind(stem + ".quarantine.", 0) == 0) {
            quarantineFound = true;
            break;
        }
    }
    EXPECT_TRUE(quarantineFound);
}

// Traceability: FR-4b (refined) | UML: FileStorage.loadNotes
TEST_F(FileStorageTest, PartiallyCorruptFile_LoadsValidRecords) {
    // Build an array: valid, malformed (missing "type"), valid, valid.
    // The malformed record sits between valid ones to prove loading continues after it.
    nlohmann::json arr = nlohmann::json::array();
    arr.push_back({{"uuid","valid-1"},{"type","text"},{"title","Note 1"},
                   {"body",""},{"createdAt",1000},{"lastModifiedAt",1000}});
    arr.push_back({{"uuid","bad"},{"title","Bad — no type field"}});   // malformed
    arr.push_back({{"uuid","valid-2"},{"type","text"},{"title","Note 2"},
                   {"body",""},{"createdAt",2000},{"lastModifiedAt",2000}});
    arr.push_back({{"uuid","valid-3"},{"type","voice"},{"title","Note 3"},
                   {"audioPath",""},{"createdAt",3000},{"lastModifiedAt",3000}});
    { std::ofstream out(tmpPath_); out << arr.dump(2); }

    NoteFactory factory;
    FileStorage storage(tmpPath_, factory);
    auto notes = storage.loadNotes();

    // All three valid records must load; the malformed one must be skipped.
    EXPECT_EQ(notes.size(), 3u);
}

// ---------------------------------------------------------------------------
// Feature B — SecureNote Passphrase Gate (FR-5, FR-5a, SPR-1, SPR-2)
// All B-series tests use MockEncryptionEngine — no real crypto linked.
// ---------------------------------------------------------------------------

// Traceability: FR-5 (refined) | UML: SecureNote.lock, SecureNote.unlock
TEST(SecureNote, CorrectPassphrase_ReturnsPlaintext) {
    MockEncryptionEngine engine;
    SecureNote note("uuid-b1", "Secure note", engine);

    EXPECT_EQ(note.lock("secret content", "testpass1"), Status::OK);

    auto result = note.unlock("testpass1");
    ASSERT_TRUE(std::holds_alternative<std::string>(result));
    EXPECT_EQ(std::get<std::string>(result), "secret content");
}

// Traceability: FR-5a (refined) | UML: SecureNote.unlock
TEST(SecureNote, WrongPassphrase_ReturnsError) {
    MockEncryptionEngine engine;
    SecureNote note("uuid-b2", "Secure note", engine);
    note.lock("secret content", "testpass1");

    auto result = note.unlock("wrongpass");
    ASSERT_TRUE(std::holds_alternative<SecureNoteError>(result));
    EXPECT_EQ(std::get<SecureNoteError>(result), SecureNoteError::WRONG_PASSPHRASE);
}

// Traceability: FR-5a (refined) | UML: SecureNote.unlock
TEST(SecureNote, EmptyPassphrase_RejectedBeforeDecrypt) {
    MockEncryptionEngine engine;
    SecureNote note("uuid-b3", "Secure note", engine);
    note.lock("secret content", "testpass1");

    // Empty passphrase must return INVALID_INPUT without calling engine_.decrypt.
    // If decrypt were called with "", mock would throw → WRONG_PASSPHRASE, not INVALID_INPUT.
    auto result = note.unlock("");
    ASSERT_TRUE(std::holds_alternative<SecureNoteError>(result));
    EXPECT_EQ(std::get<SecureNoteError>(result), SecureNoteError::INVALID_INPUT);
}

// Traceability: FR-5 (refined), FR-5a (refined) | UML: SecureNote.unlock
TEST(SecureNote, ShortPassphrase_ReturnsInvalidInput) {
    MockEncryptionEngine engine;
    SecureNote note("uuid-b4", "Secure note", engine);
    note.lock("secret content", "testpass1");

    // 7 chars — one short of the 8-char minimum (FR-5).
    auto result = note.unlock("short12");
    ASSERT_TRUE(std::holds_alternative<SecureNoteError>(result));
    EXPECT_EQ(std::get<SecureNoteError>(result), SecureNoteError::INVALID_INPUT);
}

// Traceability: FR-5 (refined), SPR-1 | UML: SecureNote.unlock
TEST(SecureNote, AfterUnlock_NoPlaintextInPublicAPI) {
    MockEncryptionEngine engine;
    SecureNote note("uuid-b5", "Secure note", engine);
    note.lock("secret content", "testpass1");

    auto result = note.unlock("testpass1");
    ASSERT_TRUE(std::holds_alternative<std::string>(result));
    EXPECT_EQ(std::get<std::string>(result), "secret content");

    // SPR-1: getCiphertext() must contain raw ciphertext bytes, not the plaintext string.
    const auto& cipher = note.getCiphertext();
    ASSERT_FALSE(cipher.empty());
    const std::string asStr(reinterpret_cast<const char*>(cipher.data()), cipher.size());
    EXPECT_EQ(asStr.find("secret content"), std::string::npos);
}

// Traceability: SPR-1 | UML: AESEngine, FileStorage, SecureNote (Integration)
TEST_F(FileStorageTest, FileStorage_CiphertextOnly) {
    AESEngine engine;
    NoteFactory factory;
    FileStorage storage(tmpPath_, factory, &engine);
    NullStorage nullStorage;
    NoteManager manager(factory, nullStorage);

    // Create and lock a SecureNote with real AES-256-GCM encryption.
    auto secureNote = std::make_unique<SecureNote>("secure-b6-uuid", "B6 note", engine);
    ASSERT_EQ(secureNote->lock("secret plaintext content", "realpassword123"), Status::OK);
    manager.add(std::move(secureNote));

    // Persist via FileStorage (atomic write).
    storage.saveAll(manager.getNotes());

    // Load the raw JSON file and inspect its contents as a string.
    std::ifstream jsonFile(tmpPath_);
    ASSERT_TRUE(jsonFile.is_open());
    const std::string content(std::istreambuf_iterator<char>(jsonFile), {});

    // SPR-1: ciphertext field must exist; no plaintext or body field.
    EXPECT_NE(content.find("\"ciphertext\""),   std::string::npos);
    EXPECT_EQ(content.find("\"body\""),         std::string::npos);
    EXPECT_EQ(content.find("\"plaintext\""),    std::string::npos);
    // The actual plaintext must not appear anywhere in the file.
    EXPECT_EQ(content.find("secret plaintext content"), std::string::npos);

    // Verify round-trip: reload and decrypt successfully.
    auto loaded = storage.loadNotes();
    ASSERT_EQ(loaded.size(), 1u);
    auto* sn = dynamic_cast<SecureNote*>(loaded[0].get());
    ASSERT_NE(sn, nullptr);
    auto unlockResult = sn->unlock("realpassword123");
    ASSERT_TRUE(std::holds_alternative<std::string>(unlockResult));
    EXPECT_EQ(std::get<std::string>(unlockResult), "secret plaintext content");
}

// Traceability: FR-4 (refined) | UML: FileStorage.loadNotes
TEST_F(FileStorageTest, DuplicateUUID_DiscardsSecond) {
    const std::string sharedUUID = "dup-uuid";
    nlohmann::json arr = nlohmann::json::array();
    arr.push_back({{"uuid",sharedUUID},{"type","text"},{"title","First"},
                   {"body","first body"},{"createdAt",1000},{"lastModifiedAt",1000}});
    arr.push_back({{"uuid",sharedUUID},{"type","text"},{"title","Second"},
                   {"body","second body"},{"createdAt",2000},{"lastModifiedAt",2000}});
    { std::ofstream out(tmpPath_); out << arr.dump(2); }

    NoteFactory factory;
    FileStorage storage(tmpPath_, factory);
    auto notes = storage.loadNotes();

    ASSERT_EQ(notes.size(), 1u);
    EXPECT_EQ(notes[0]->getTitle(), "First"); // FR-4: first record wins
}
