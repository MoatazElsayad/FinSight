#pragma once

#include "Common.h"

#include <string>

namespace finsight::core::models {

struct Budget {
    std::string id;
    std::string userId;
    std::string categoryId;
    YearMonth period;
    double limit {0.0};
};

struct BudgetStatus {
    Budget budget;
    double spent {0.0};
    double remaining {0.0};
    bool overspent {false};
    double usageRatio {0.0};
};

}  // namespace finsight::core::models
