#pragma once

#include "Common.h"

#include <string>
#include <vector>

using namespace std;

namespace finsight::core::models {

// Identifies whether a transaction is income or expense.
enum class TransactionType {
    Income,
    Expense
};

// Stores one finance transaction.
struct Transaction {
    std::string id;
    std::string userId;
    std::string title;
    std::string description;
    std::string categoryId;
    TransactionType type {TransactionType::Expense};
    double amount {0.0};
    Date date;
    std::string merchant;
    std::vector<std::string> tags;
};

// Describes the filters used when browsing transaction history.
struct TransactionFilter {
    std::optional<Date> from;
    std::optional<Date> to;
    std::optional<std::string> categoryId;
    std::optional<TransactionType> type;
    std::optional<std::string> searchText;
};

}  // namespace finsight::core::models
