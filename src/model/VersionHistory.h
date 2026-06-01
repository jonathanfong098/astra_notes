#pragma once
// Traceability: NFR-3 (refined) | UML: VersionHistory

#include "VersionEntry.h"
#include <vector>

class VersionHistory {
public:
    VersionHistory() = default;

    // Appends a snapshot entry after each body edit.
    // Traceability: NFR-3 (refined) | UML: VersionHistory.addEntry
    void addEntry(VersionEntry entry);

    // Returns all version entries in insertion order.
    // Traceability: NFR-3 (refined) | UML: VersionHistory.getEntries
    const std::vector<VersionEntry>& getEntries() const;

private:
    std::vector<VersionEntry> entries_;
};
