// Traceability: FR-3 (refined stub) | UML: VoiceNote
#include "VoiceNote.h"
#include <utility>

// Traceability: FR-3 (refined stub) | UML: VoiceNote.VoiceNote
VoiceNote::VoiceNote(UUID uuid, std::string title)
    : Note(std::move(uuid), std::move(title)) {}

// Traceability: FR-4 (refined) | UML: VoiceNote.VoiceNote
VoiceNote::VoiceNote(UUID uuid, std::string title, std::string audioPath,
                     std::time_t createdAt, std::time_t lastModifiedAt)
    : Note(std::move(uuid), std::move(title), createdAt, lastModifiedAt)
    , audioPath_(std::move(audioPath)) {}

// Traceability: FR-3 (refined stub) | UML: VoiceNote.getType
std::string VoiceNote::getType() const { return "voice"; }

// Traceability: FR-3 (refined) | UML: VoiceNote.getAudioPath
const std::string& VoiceNote::getAudioPath() const { return audioPath_; }

// Traceability: FR-2 (refined) | UML: VoiceNote.setAudioPath
void VoiceNote::setAudioPath(std::string path) { audioPath_ = std::move(path); }

// Traceability: FR-2 (refined) | UML: VoiceNote.getBody
std::string VoiceNote::getBody() const { return audioPath_; }
