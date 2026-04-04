#pragma once

#include "../../core/managers/FinanceTrackerBackend.h"

#include <filesystem>

using namespace std;

namespace finsight::data::storage {

class BackendStore {
public:
    // Writes the complete backend state to the given directory.
    void save(const core::managers::FinanceTrackerBackend& backend,
              const filesystem::path& directory) const;
    // Restores the complete backend state from the given directory.
    void load(core::managers::FinanceTrackerBackend& backend,
              const filesystem::path& directory) const;
    // Returns the SQLite database file used for core finance data.
    filesystem::path databasePath(const filesystem::path& directory) const;
    // Returns the JSON sidecar file used for flexible app state.
    filesystem::path sidecarPath(const filesystem::path& directory) const;
};

}  // namespace finsight::data::storage
