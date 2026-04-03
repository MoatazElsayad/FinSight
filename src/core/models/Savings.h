#pragma once

#include "Common.h"

#include <string>

namespace finsight::core::models {

enum class SavingsEntryType {
    Deposit,
    Withdrawal
};

struct SavingsEntry {
    std::string id;
    std::string userId;
    SavingsEntryType type {SavingsEntryType::Deposit};
    double amount {0.0};
    Date date;
    std::string note;
};

struct SavingsGoal {
    std::string id;
    std::string userId;
    double monthlyTarget {0.0};
    double longTermTarget {0.0};
    Date targetDate;
};

struct SavingsOverview {
    double currentBalance {0.0};
    double monthlySaved {0.0};
    double monthlyTarget {0.0};
    double progressToMonthlyTarget {0.0};
    double longTermTarget {0.0};
};

}  // namespace finsight::core::models
