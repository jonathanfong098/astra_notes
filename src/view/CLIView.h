#pragma once
// Traceability: FR-1 (refined), Lab-vs-UML reconciliation | UML: CLIView

#include "../controller/NoteManager.h"
#include <string>

class CLIView {
public:
    // Traceability: FR-1 (refined) | UML: CLIView.CLIView
    explicit CLIView(NoteManager& manager);

    // Prints a numbered list of all notes (index, UUID, type, title) to stdout.
    // Traceability: FR-1 (refined) | UML: CLIView.renderNoteList
    void renderNoteList() const;

    // Prints full details for the note with the given UUID. Prints error if not found.
    // Traceability: FR-1 (refined) | UML: CLIView.renderNote
    void renderNote(const UUID& uuid) const;

    // Displays prompt string (no newline), reads one line from stdin, trims whitespace.
    // Traceability: FR-1 (refined) | UML: CLIView.promptInput
    std::string promptInput(const std::string& prompt) const;

private:
    NoteManager& manager_; // injected; not owned
};
