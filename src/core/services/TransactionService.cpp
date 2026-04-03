#include "TransactionService.h"

#include <algorithm>
#include <regex>
#include <sstream>
#include <stdexcept>

namespace finsight::core::services {

namespace {

bool filterMatches(const models::Transaction& transaction,
                   const models::TransactionFilter& filter) {
    if (filter.from && transaction.date < *filter.from) {
        return false;
    }
    if (filter.to && transaction.date > *filter.to) {
        return false;
    }
    if (filter.categoryId && transaction.categoryId != *filter.categoryId) {
        return false;
    }
    if (filter.type && transaction.type != *filter.type) {
        return false;
    }
    if (filter.searchText &&
        !models::containsCaseInsensitive(transaction.title, *filter.searchText) &&
        !models::containsCaseInsensitive(transaction.description, *filter.searchText) &&
        !models::containsCaseInsensitive(transaction.merchant, *filter.searchText)) {
        return false;
    }
    return true;
}

}  // namespace

TransactionService::TransactionService() {
    createCategory("", "Salary", models::CategoryKind::Income, "briefcase", true);
    createCategory("", "Freelance", models::CategoryKind::Income, "laptop", true);
    createCategory("", "Food", models::CategoryKind::Expense, "fork-knife", true);
    createCategory("", "Transport", models::CategoryKind::Expense, "car", true);
    createCategory("", "Bills", models::CategoryKind::Expense, "receipt", true);
    createCategory("", "Savings", models::CategoryKind::Savings, "piggy-bank", true);
    createCategory("", "Investments", models::CategoryKind::Investment, "chart-line", true);
}

const std::vector<models::Category>& TransactionService::getCategories() const {
    return categories_;
}

std::vector<models::Category> TransactionService::getCategoriesForUser(const std::string& userId) const {
    std::vector<models::Category> result;
    for (const auto& category : categories_) {
        if (category.builtIn || category.userId == userId) {
            result.push_back(category);
        }
    }
    return result;
}

models::Category TransactionService::createCategory(const std::string& userId,
                                                    const std::string& name,
                                                    models::CategoryKind kind,
                                                    const std::string& icon,
                                                    bool builtIn) {
    if (name.empty()) {
        throw std::invalid_argument("Category name is required.");
    }
    models::Category category {
        .id = nextId("cat"),
        .userId = userId,
        .name = name,
        .icon = icon,
        .kind = kind,
        .builtIn = builtIn,
        .archived = false,
    };
    categories_.push_back(category);
    return category;
}

void TransactionService::deleteCategory(const std::string& userId, const std::string& categoryId) {
    auto iterator = std::find_if(categories_.begin(), categories_.end(), [&](const auto& category) {
        return category.id == categoryId && !category.builtIn && category.userId == userId;
    });
    if (iterator == categories_.end()) {
        throw std::out_of_range("Custom category not found.");
    }
    categories_.erase(iterator);
}

models::Transaction TransactionService::addTransaction(models::Transaction transaction) {
    ensureTransactionIsValid(transaction);
    transaction.id = nextId("txn");
    transactions_.push_back(transaction);
    return transaction;
}

models::Transaction TransactionService::updateTransaction(const std::string& userId,
                                                          const std::string& transactionId,
                                                          const models::Transaction& updated) {
    for (auto& transaction : transactions_) {
        if (transaction.id == transactionId && transaction.userId == userId) {
            models::Transaction candidate = updated;
            candidate.id = transaction.id;
            candidate.userId = userId;
            ensureTransactionIsValid(candidate);
            transaction = candidate;
            return transaction;
        }
    }
    throw std::out_of_range("Transaction not found.");
}

void TransactionService::deleteTransaction(const std::string& userId, const std::string& transactionId) {
    auto iterator = std::find_if(transactions_.begin(), transactions_.end(), [&](const auto& transaction) {
        return transaction.id == transactionId && transaction.userId == userId;
    });
    if (iterator == transactions_.end()) {
        throw std::out_of_range("Transaction not found.");
    }
    transactions_.erase(iterator);
}

void TransactionService::bulkDeleteTransactions(const std::string& userId,
                                                const std::vector<std::string>& transactionIds) {
    transactions_.erase(std::remove_if(transactions_.begin(),
                                       transactions_.end(),
                                       [&](const auto& transaction) {
                                           return transaction.userId == userId &&
                                                  std::find(transactionIds.begin(),
                                                            transactionIds.end(),
                                                            transaction.id) != transactionIds.end();
                                       }),
                        transactions_.end());
}

std::vector<models::Transaction> TransactionService::listTransactions(const std::string& userId) const {
    std::vector<models::Transaction> result;
    for (const auto& transaction : transactions_) {
        if (transaction.userId == userId) {
            result.push_back(transaction);
        }
    }
    std::sort(result.begin(), result.end(), [](const auto& left, const auto& right) {
        return left.date > right.date;
    });
    return result;
}

std::vector<models::Transaction> TransactionService::filterTransactions(
    const std::string& userId,
    const models::TransactionFilter& filter) const {
    std::vector<models::Transaction> result;
    for (const auto& transaction : transactions_) {
        if (transaction.userId == userId && filterMatches(transaction, filter)) {
            result.push_back(transaction);
        }
    }
    std::sort(result.begin(), result.end(), [](const auto& left, const auto& right) {
        return left.date > right.date;
    });
    return result;
}

double TransactionService::sumTransactions(const std::string& userId,
                                           models::TransactionType type,
                                           const models::YearMonth& period) const {
    double total = 0.0;
    for (const auto& transaction : transactions_) {
        if (transaction.userId == userId &&
            transaction.type == type &&
            models::inMonth(transaction.date, period)) {
            total += transaction.amount;
        }
    }
    return total;
}

double TransactionService::spentForCategory(const std::string& userId,
                                            const std::string& categoryId,
                                            const models::YearMonth& period) const {
    double total = 0.0;
    for (const auto& transaction : transactions_) {
        if (transaction.userId == userId &&
            transaction.categoryId == categoryId &&
            transaction.type == models::TransactionType::Expense &&
            models::inMonth(transaction.date, period)) {
            total += transaction.amount;
        }
    }
    return total;
}

const models::Category& TransactionService::requireCategory(const std::string& categoryId) const {
    for (const auto& category : categories_) {
        if (category.id == categoryId) {
            return category;
        }
    }
    throw std::out_of_range("Category not found.");
}

std::vector<models::Transaction> TransactionService::allTransactions() const {
    return transactions_;
}

void TransactionService::loadState(std::vector<models::Category> categories,
                                   std::vector<models::Transaction> transactions) {
    categories_ = std::move(categories);
    transactions_ = std::move(transactions);

    std::size_t maxCategoryId = 0;
    std::size_t maxTransactionId = 0;
    const std::regex categoryPattern(R"(cat-(\d+))");
    const std::regex transactionPattern(R"(txn-(\d+))");

    for (const auto& category : categories_) {
        std::smatch match;
        if (std::regex_match(category.id, match, categoryPattern)) {
            maxCategoryId = std::max<std::size_t>(maxCategoryId, static_cast<std::size_t>(std::stoull(match[1].str())));
        }
    }

    for (const auto& transaction : transactions_) {
        std::smatch match;
        if (std::regex_match(transaction.id, match, transactionPattern)) {
            maxTransactionId = std::max<std::size_t>(maxTransactionId, static_cast<std::size_t>(std::stoull(match[1].str())));
        }
    }

    nextCategoryId_ = maxCategoryId + 1;
    nextTransactionId_ = maxTransactionId + 1;
}

std::string TransactionService::nextId(const std::string& prefix) {
    std::ostringstream stream;
    if (prefix == "cat") {
        stream << prefix << '-' << nextCategoryId_++;
    } else {
        stream << prefix << '-' << nextTransactionId_++;
    }
    return stream.str();
}

void TransactionService::ensureTransactionIsValid(const models::Transaction& transaction) const {
    if (transaction.userId.empty() || transaction.title.empty() || transaction.amount <= 0.0) {
        throw std::invalid_argument("Transaction requires user, title, and positive amount.");
    }

    const auto& category = requireCategory(transaction.categoryId);
    if (!category.builtIn && category.userId != transaction.userId) {
        throw std::invalid_argument("Transaction category does not belong to the user.");
    }
}

}  // namespace finsight::core::services
