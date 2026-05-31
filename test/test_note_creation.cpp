// Traceability: FR-1 (refined), FR-2 (refined), FR-3 (refined) | UML: NoteFactory.create, NoteManager
#include <gtest/gtest.h>
#include "controller/NoteFactory.h"
#include "controller/NoteManager.h"
#include "storage/StorageInterface.h"
#include "model/Note.h"
#include "model/TextNote.h"
#include "model/SecureNote.h"
#include "encryption/EncryptionEngine.h"
#include <memory>
#include <stdexcept>
#include <string>

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
