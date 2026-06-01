// Traceability: NFR-3 (refined) | UML: VersionHistory
#include "VersionHistory.h"
#include <utility>

// Traceability: NFR-3 (refined) | UML: VersionHistory.addEntry
void VersionHistory::addEntry(VersionEntry entry) {
    entries_.push_back(std::move(entry));
}

// Traceability: NFR-3 (refined) | UML: VersionHistory.getEntries
const std::vector<VersionEntry>& VersionHistory::getEntries() const {
    return entries_;
}
