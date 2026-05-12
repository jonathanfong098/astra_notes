#pragma once
// Traceability: NFR-3 (refined stub) | UML: VersionHistory

#include "VersionEntry.h"
#include <vector>

class VersionHistory {
public:
    VersionHistory() = default;

    // Returns all version entries; always empty this sprint.
    // Traceability: NFR-3 (refined stub) | UML: VersionHistory.getEntries
    const std::vector<VersionEntry>& getEntries() const;

private:
    std::vector<VersionEntry> entries_; // never populated this sprint
};
