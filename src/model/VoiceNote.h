#pragma once
// Traceability: FR-3 (refined stub) | UML: VoiceNote

#include "Note.h"
#include <string>

class VoiceNote : public Note {
public:
    VoiceNote(UUID uuid, std::string title);

    // Reconstruction constructor — restores from persisted JSON record via NoteFactory::reconstructRecord.
    // Traceability: FR-4 (refined) | UML: VoiceNote.VoiceNote
    VoiceNote(UUID uuid, std::string title, std::string audioPath,
              std::time_t createdAt, std::time_t lastModifiedAt);

    // Traceability: FR-3 (refined stub) | UML: VoiceNote.getType
    std::string getType() const override;

    // Returns audio file path (empty until set).
    // Traceability: FR-3 (refined) | UML: VoiceNote.getAudioPath
    const std::string& getAudioPath() const;

    // Sets the audio file path.
    // Traceability: FR-2 (refined) | UML: VoiceNote.setAudioPath
    void setAudioPath(std::string path);

    // Returns audioPath_ so CLIView::renderNote() can display it via Note::getBody().
    // Traceability: FR-2 (refined) | UML: VoiceNote.getBody
    std::string getBody() const override;

private:
    std::string audioPath_; // always empty this sprint — FR-3 deferred
};
