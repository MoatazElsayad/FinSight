#pragma once

#include "Common.h"

#include <string>

using namespace std;

namespace finsight::core::models {

// Identifies whether a savings entry is money in or money out.
enum class SavingsEntryType {
    Deposit,
    Withdrawal
};

// Stores one deposit or withdrawal in savings.
struct SavingsEntry {
    std::string id;
    std::string userId;
    SavingsEntryType type {SavingsEntryType::Deposit};
    double amount {0.0};
    Date date;
    std::string note;
};

// Stores the user's monthly and long-term savings targets.
struct SavingsGoal {
    std::string id;
    std::string userId;
    double monthlyTarget {0.0};
    double longTermTarget {0.0};
    Date targetDate;
};

// Summarizes the user's current savings position.
struct SavingsOverview {
    double currentBalance {0.0};
    double monthlySaved {0.0};
    double monthlyTarget {0.0};
    double progressToMonthlyTarget {0.0};
    double longTermTarget {0.0};
};

}  // namespace finsight::core::models
