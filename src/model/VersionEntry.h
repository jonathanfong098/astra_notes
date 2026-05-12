#pragma once
// Traceability: NFR-3 (refined stub) | UML: VersionEntry

#include <string>
#include <ctime>

struct VersionEntry {
    std::string snapshotBody;
    std::time_t timestamp{0};
};
