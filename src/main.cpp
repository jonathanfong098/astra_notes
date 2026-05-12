// Traceability: FR-1 (refined), Lab-vs-UML reconciliation | UML: main
// Lab-vs-UML: profile/settings implemented as "[s] stub" per docs/ai-reflection.md.
// No SettingsManager or ProfileService created — neither appears in the UML.

#include "controller/NoteFactory.h"
#include "controller/NoteManager.h"
#include "storage/StorageInterface.h"
#include "view/CLIView.h"
#include "model/Note.h"
#include <iostream>
#include <memory>
#include <vector>

// Minimal no-op storage for this sprint.
struct NullStorage final : public StorageInterface {
    void saveNote(const Note&) override {}
    std::vector<std::unique_ptr<Note>> loadNotes() override { return {}; }
};

int main() {
    NoteFactory factory;
    NullStorage storage;
    NoteManager manager(factory, storage);
    CLIView view(manager);

    std::cout << "AstraNotes v0.1\n"
              << "Commands: [n] new note  [l] list notes  [s] settings  [q] quit\n\n";

    std::string cmd;
    while (true) {
        cmd = view.promptInput("> ");

        if (cmd == "n") {
            std::string title = view.promptInput("Title: ");
            try {
                auto note = factory.create("text", title);
                manager.add(std::move(note));
                std::cout << "Note created.\n";
            } catch (const std::invalid_argument& e) {
                std::cout << "Error: " << e.what() << '\n';
            }

        } else if (cmd == "l") {
            view.renderNoteList();

        } else if (cmd == "s") {
            // Lab-vs-UML reconciliation: CLIView UML has no settings method.
            // Profile and settings satisfied as a stub per docs/ai-reflection.md.
            std::cout << "Settings: not yet implemented.\n";

        } else if (cmd == "q") {
            std::cout << "Goodbye.\n";
            break;

        } else if (!cmd.empty()) {
            std::cout << "Unknown command. Use n, l, s, or q.\n";
        }
    }
    return 0;
}
