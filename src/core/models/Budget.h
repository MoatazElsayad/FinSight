#pragma once

#include "Common.h"

#include <string>
#include <vector>

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

// Compares one category budget between two periods.
struct BudgetComparisonLine {
    std::string categoryId;
    std::string categoryName;
    double currentLimit {0.0};
    double previousLimit {0.0};
    double currentSpent {0.0};
    double previousSpent {0.0};
    double limitDelta {0.0};
    double spentDelta {0.0};
};

// Stores month-over-month budget comparison data.
struct BudgetPeriodComparison {
    YearMonth currentPeriod;
    YearMonth previousPeriod;
    double currentTotalLimit {0.0};
    double previousTotalLimit {0.0};
    double currentTotalSpent {0.0};
    double previousTotalSpent {0.0};
    double totalLimitDelta {0.0};
    double totalSpentDelta {0.0};
    std::vector<BudgetComparisonLine> categories;
};

}  // namespace finsight::core::models
