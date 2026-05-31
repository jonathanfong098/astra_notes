// Traceability: FR-1 (refined) | UML: NoteFactory.create, NoteManager.add
#include <gtest/gtest.h>
#include "controller/NoteFactory.h"
#include "controller/NoteManager.h"
#include "storage/StorageInterface.h"
#include "model/Note.h"
#include <memory>
#include <stdexcept>
#include <string>

// Minimal no-op StorageInterface for unit tests — no file I/O.
class NullStorage final : public StorageInterface {
public:
    void saveNote(const Note&) override {}
    std::vector<std::unique_ptr<Note>> loadNotes() override { return {}; }
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
