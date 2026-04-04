#pragma once

#include "Common.h"

#include <string>

using namespace std;

namespace finsight::core::models {

// Stores a monthly spending budget for one category.
struct Budget {
    std::string id;
    std::string userId;
    std::string categoryId;
    YearMonth period;
    double limit {0.0};
};

// Summarizes how a budget is performing against actual spending.
struct BudgetStatus {
    Budget budget;
    double spent {0.0};
    double remaining {0.0};
    bool overspent {false};
    double usageRatio {0.0};
};

}  // namespace finsight::core::models
