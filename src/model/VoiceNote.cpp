// Traceability: FR-3 (refined stub) | UML: VoiceNote
#include "VoiceNote.h"
#include <utility>

// Traceability: FR-3 (refined stub) | UML: VoiceNote.VoiceNote
VoiceNote::VoiceNote(UUID uuid, std::string title)
    : Note(std::move(uuid), std::move(title)) {}

// Traceability: FR-3 (refined stub) | UML: VoiceNote.getType
std::string VoiceNote::getType() const { return "voice"; }

// Traceability: FR-3 (refined stub) | UML: VoiceNote.getAudioPath
const std::string& VoiceNote::getAudioPath() const { return audioPath_; }
