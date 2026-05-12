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
