// Traceability: FR-1 (refined), Lab-vs-UML reconciliation | UML: CLIView
#include "CLIView.h"
#include <iostream>
#include <iomanip>
#include <ctime>
#include <algorithm>
#include <cctype>

// Traceability: FR-1 (refined) | UML: CLIView.CLIView
CLIView::CLIView(NoteManager& manager) : manager_(manager) {}

// Traceability: FR-1 (refined) | UML: CLIView.renderNoteList
void CLIView::renderNoteList() const {
    const auto& notes = manager_.getNotes();
    if (notes.empty()) {
        std::cout << "(no notes)\n";
        return;
    }
    int idx = 1;
    for (const auto& [uuid, note] : notes) {
        std::cout << '[' << idx++ << "] "
                  << uuid << "  "
                  << note->getType() << "  "
                  << note->getTitle() << '\n';
    }
}

// Traceability: FR-1 (refined) | UML: CLIView.renderNote
void CLIView::renderNote(const UUID& uuid) const {
    const Note* note = manager_.findByUUID(uuid);
    if (!note) {
        std::cout << "Note not found: " << uuid << '\n';
        return;
    }
    std::time_t t = note->getCreatedAt();
    char timebuf[32]{};
    std::strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M:%S", std::localtime(&t));
    std::cout << "UUID:    " << note->getUUID()  << '\n'
              << "Type:    " << note->getType()  << '\n'
              << "Title:   " << note->getTitle() << '\n'
              << "Created: " << timebuf          << '\n';
    const std::string body = note->getBody();
    if (!body.empty())
        std::cout << "Body:\n" << body << '\n';
}

// Traceability: FR-1 (refined) | UML: CLIView.promptInput
std::string CLIView::promptInput(const std::string& prompt) const {
    std::cout << prompt << std::flush;
    std::string line;
    std::getline(std::cin, line);
    auto start = std::find_if_not(line.begin(), line.end(),
        [](unsigned char c){ return std::isspace(c); });
    auto end = std::find_if_not(line.rbegin(), line.rend(),
        [](unsigned char c){ return std::isspace(c); }).base();
    return (start < end) ? std::string(start, end) : std::string{};
}
