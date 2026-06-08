// Traceability: FR-1, FR-2, FR-3, FR-4, FR-5, Lab-vs-UML reconciliation | UML: main
// Lab-vs-UML: [s] settings is a stub per docs/ai-reflection.md — no SettingsManager exists.

#include "controller/NoteFactory.h"
#include "controller/NoteManager.h"
#include "storage/FileStorage.h"
#include "encryption/AESEngine.h"
#include "model/Note.h"
#include "model/SecureNote.h"
#include "view/CLIView.h"

#include <iostream>
#include <memory>
#include <variant>
#include <filesystem>
#include <cstdlib>

namespace {

// Resolves ~/.astranotes/notes.json, creating the directory if needed (FR-4a).
// Traceability: FR-4 (refined) | UML: FileStorage
std::string resolveStoragePath() {
    const char* home = std::getenv("HOME");
    const std::string dir = home ? std::string(home) + "/.astranotes" : ".astranotes";
    std::filesystem::create_directories(dir);
    return dir + "/notes.json";
}

} // anonymous namespace

int main() {
    AESEngine   engine;
    NoteFactory factory;
    const std::string storagePath = resolveStoragePath();
    FileStorage storage(storagePath, factory, &engine);
    NoteManager manager(factory, storage);
    CLIView     view(manager);

    manager.loadAll(); // FR-4: deserialise notes from disk on startup

    std::cout << "AstraNotes v0.2  [" << storagePath << "]\n"
              << "Commands: [n]ew  [l]ist  [v]iew  [e]dit  [d]elete  [s]ettings  [h]elp  [q]uit\n\n";

    std::string cmd;
    while (true) {
        cmd = view.promptInput("> ");

        // ── [n] New note ────────────────────────────────────────────────
        if (cmd == "n") {
            const std::string type  = view.promptInput("Type (text/voice/secure): ");
            const std::string title = view.promptInput("Title: ");

            if (type == "text" || type == "voice") {
                try {
                    manager.add(factory.create(type, title));
                    manager.persistAll();
                    std::cout << "Note created.\n";
                } catch (const std::exception& e) {
                    std::cout << "Error: " << e.what() << '\n';
                }
            } else if (type == "secure") {
                // Traceability: FR-1a (refined) | UML: NoteFactory.create
                // CLIView::promptInput already trims whitespace, so empty == whitespace-only.
                if (title.empty()) {
                    std::cout << "Error: note title must not be empty.\n";
                } else if (title.size() > 255) {
                    std::cout << "Error: note title must not exceed 255 characters.\n";
                } else {
                    // Prompt passphrase before content so a short passphrase fails fast
                    // without discarding the content the user already typed (FR-5).
                    const std::string pass = view.promptInput("Passphrase (min 8 chars): ");
                    if (pass.size() < 8) {
                        std::cout << "Error: passphrase must be at least 8 characters.\n";
                    } else {
                        const std::string body = view.promptInput("Content: ");
                        try {
                            // Traceability: FR-1 (refined) | UML: NoteFactory.generateUUID
                            auto note = std::make_unique<SecureNote>(factory.generateUUID(), title, engine);
                            if (note->lock(body, pass) != Status::OK) {
                                std::cout << "Error: encryption failed.\n";
                            } else {
                                manager.add(std::move(note));
                                manager.persistAll();
                                std::cout << "Secure note created and locked.\n";
                            }
                        } catch (const std::exception& e) {
                            std::cout << "Error: " << e.what() << '\n';
                        }
                    }
                }
            } else {
                std::cout << "Unknown type. Use text, voice, or secure.\n";
            }

        // ── [l] List notes ──────────────────────────────────────────────
        } else if (cmd == "l") {
            view.renderNoteList();

        // ── [v] View note ───────────────────────────────────────────────
        } else if (cmd == "v") {
            const std::string uuid = view.promptInput("UUID: ");
            const Note* note = manager.findByUUID(uuid);
            if (!note) {
                std::cout << "Note not found.\n";
            } else {
                view.renderNote(uuid);
                if (note->getType() == "secure") {
                    const std::string pass = view.promptInput("Passphrase to unlock: ");
                    const auto& sn = static_cast<const SecureNote&>(*note);
                    auto result = sn.unlock(pass); // unlock() is const — no cast needed
                    if (std::holds_alternative<std::string>(result)) {
                        std::cout << "Content: " << std::get<std::string>(result) << '\n';
                    } else {
                        const auto err = std::get<SecureNoteError>(result);
                        std::cout << (err == SecureNoteError::WRONG_PASSPHRASE
                            ? "Error: wrong passphrase.\n"
                            : "Error: invalid passphrase (min 8 chars).\n");
                    }
                }
            }

        // ── [e] Edit note ───────────────────────────────────────────────
        } else if (cmd == "e") {
            const std::string uuid  = view.promptInput("UUID: ");
            const std::string field = view.promptInput("Field (title/body): ");

            if (field != "title" && field != "body") {
                std::cout << "Unknown field. Use title or body.\n";
                continue;
            }

            const Note* note = manager.findByUUID(uuid);
            if (!note) { std::cout << "Error: note not found.\n"; continue; }

            // SecureNote body edit: prompt passphrase and re-lock with new content.
            if (field == "body" && note->getType() == "secure") {
                const std::string pass    = view.promptInput("Passphrase: ");
                const std::string newBody = view.promptInput("New content: ");
                const Status s = manager.relockSecureBody(uuid, newBody, pass);
                if      (s == Status::OK)            { manager.persistAll(); std::cout << "Secure note updated.\n"; }
                else if (s == Status::NOT_FOUND)     std::cout << "Error: note not found.\n";
                else if (s == Status::INVALID_INPUT) std::cout << "Error: passphrase must be at least 8 characters.\n";
            } else {
                const std::string value = view.promptInput("New value: ");
                Status s;
                if (field == "title") s = manager.editTitle(uuid, value);
                else                  s = manager.editBody(uuid, value);

                if      (s == Status::OK)            { manager.persistAll(); std::cout << "Note updated.\n"; }
                else if (s == Status::NOT_FOUND)     std::cout << "Error: note not found.\n";
                else if (s == Status::INVALID_INPUT) std::cout << "Error: invalid input.\n";
            }

        // ── [d] Delete note ─────────────────────────────────────────────
        } else if (cmd == "d") {
            const std::string uuid    = view.promptInput("UUID: ");
            const std::string confirm = view.promptInput("Confirm delete? (yes/no): ");
            if (confirm == "yes") {
                const Status s = manager.remove(uuid);
                if      (s == Status::OK)        { manager.persistAll(); std::cout << "Note deleted.\n"; }
                else if (s == Status::NOT_FOUND) std::cout << "Error: note not found.\n";
            } else {
                std::cout << "Delete cancelled.\n";
            }

        // ── [h] Help ────────────────────────────────────────────────────
        } else if (cmd == "h" || cmd == "help") {
            std::cout
                << "Commands:\n"
                << "  n  new note    — prompts for type (text/voice/secure) and title\n"
                << "  l  list notes  — shows all notes (UUID, type, title)\n"
                << "  v  view note   — prompts for UUID; unlocks SecureNote with passphrase\n"
                << "  e  edit note   — prompts for UUID, field (title/body), new value\n"
                << "  d  delete note — prompts for UUID and confirmation\n"
                << "  s  settings    — not yet implemented\n"
                << "  h  help        — show this message\n"
                << "  q  quit        — saves all notes and exits\n";

        // ── [s] Settings (stub) ─────────────────────────────────────────
        } else if (cmd == "s") {
            // Lab-vs-UML reconciliation: CLIView UML has no settings method.
            // Profile and settings satisfied as a stub per docs/ai-reflection.md.
            std::cout << "Settings: not yet implemented.\n";

        // ── [q] Quit ────────────────────────────────────────────────────
        } else if (cmd == "q") {
            manager.persistAll(); // FR-4: serialise all notes to disk on exit
            std::cout << "Notes saved. Goodbye.\n";
            break;

        } else if (!cmd.empty()) {
            std::cout << "Unknown command. Type h for help.\n";
        }
    }
    return 0;
}
