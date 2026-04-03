#pragma once

#include "Common.h"

#include <string>
#include <vector>

namespace finsight::core::models {

enum class TransactionType {
    Income,
    Expense
};

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

struct TransactionFilter {
    std::optional<Date> from;
    std::optional<Date> to;
    std::optional<std::string> categoryId;
    std::optional<TransactionType> type;
    std::optional<std::string> searchText;
};

}  // namespace finsight::core::models
