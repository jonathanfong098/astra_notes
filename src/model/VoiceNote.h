#pragma once
// Traceability: FR-3 (refined stub) | UML: VoiceNote

#include "Note.h"
#include <string>

class VoiceNote : public Note {
public:
    VoiceNote(UUID uuid, std::string title);

    // Traceability: FR-3 (refined stub) | UML: VoiceNote.getType
    std::string getType() const override;

    // Returns audio file path; always empty this sprint.
    // Traceability: FR-3 (refined stub) | UML: VoiceNote.getAudioPath
    const std::string& getAudioPath() const;

private:
    std::string audioPath_; // always empty this sprint — FR-3 deferred
};
