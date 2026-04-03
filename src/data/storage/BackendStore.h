#pragma once

#include "../../core/managers/FinanceTrackerBackend.h"

#include <filesystem>

namespace finsight::data::storage {

class BackendStore {
public:
    void save(const core::managers::FinanceTrackerBackend& backend,
              const std::filesystem::path& directory) const;
    void load(core::managers::FinanceTrackerBackend& backend,
              const std::filesystem::path& directory) const;
};

}  // namespace finsight::data::storage
