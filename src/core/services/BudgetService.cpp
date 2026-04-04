#include "BudgetService.h"

#include "TransactionService.h"

#include <algorithm>
#include <regex>
#include <sstream>
#include <stdexcept>

using namespace std;

namespace finsight::core::services {

// Creates and stores a new monthly budget.
models::Budget BudgetService::createBudget(const std::string& userId,
                                           const std::string& categoryId,
                                           const models::YearMonth& period,
                                           double limit) {
    if (userId.empty() || categoryId.empty() || limit <= 0.0) {
        throw std::invalid_argument("Budget requires user, category, and positive limit.");
    }
    models::Budget budget {
        .id = nextId(),
        .userId = userId,
        .categoryId = categoryId,
        .period = period,
        .limit = limit,
    };
    budgets_.push_back(budget);
    return budget;
}

// Updates one stored budget.
models::Budget BudgetService::updateBudget(const std::string& userId,
                                           const std::string& budgetId,
                                           const models::Budget& updatedBudget) {
    for (auto& budget : budgets_) {
        if (budget.id == budgetId && budget.userId == userId) {
            budget.categoryId = updatedBudget.categoryId;
            budget.period = updatedBudget.period;
            budget.limit = updatedBudget.limit;
            return budget;
        }
    }
    throw std::out_of_range("Budget not found.");
}

// Deletes a budget by id.
void BudgetService::deleteBudget(const std::string& userId, const std::string& budgetId) {
    auto iterator = std::find_if(budgets_.begin(), budgets_.end(), [&](const auto& budget) {
        return budget.id == budgetId && budget.userId == userId;
    });
    if (iterator == budgets_.end()) {
        throw std::out_of_range("Budget not found.");
    }
    budgets_.erase(iterator);
}

// Copies all budgets from one month into another month.
void BudgetService::copyBudgets(const std::string& userId,
                                const models::YearMonth& from,
                                const models::YearMonth& to) {
    const auto sourceBudgets = listBudgets(userId, from);
    for (const auto& budget : sourceBudgets) {
        createBudget(userId, budget.categoryId, to, budget.limit);
    }
}

// Returns all budgets for the selected user and month.
std::vector<models::Budget> BudgetService::listBudgets(const std::string& userId,
                                                       const models::YearMonth& period) const {
    std::vector<models::Budget> result;
    for (const auto& budget : budgets_) {
        if (budget.userId == userId && budget.period == period) {
            result.push_back(budget);
        }
    }
    return result;
}

// Builds spent/remaining summaries for every budget in a month.
std::vector<models::BudgetStatus> BudgetService::summarizeBudgets(
    const std::string& userId,
    const models::YearMonth& period,
    const TransactionService& transactionService) const {
    std::vector<models::BudgetStatus> result;
    for (const auto& budget : listBudgets(userId, period)) {
        const double spent = transactionService.spentForCategory(userId, budget.categoryId, period);
        const double remaining = budget.limit - spent;
        result.push_back(models::BudgetStatus {
            .budget = budget,
            .spent = spent,
            .remaining = remaining,
            .overspent = spent > budget.limit,
            .usageRatio = budget.limit <= 0.0 ? 0.0 : spent / budget.limit,
        });
    }
    return result;
}

// Returns every stored budget.
std::vector<models::Budget> BudgetService::allBudgets() const {
    return budgets_;
}

// Restores budgets from persisted state and resets id generation.
void BudgetService::loadBudgets(std::vector<models::Budget> budgets) {
    budgets_ = std::move(budgets);
    std::size_t maxId = 0;
    const std::regex pattern(R"(bdg-(\d+))");
    for (const auto& budget : budgets_) {
        std::smatch match;
        if (std::regex_match(budget.id, match, pattern)) {
            maxId = std::max<std::size_t>(maxId, static_cast<std::size_t>(std::stoull(match[1].str())));
        }
    }
    nextBudgetId_ = maxId + 1;
}

// Builds the next budget id string.
std::string BudgetService::nextId() {
    std::ostringstream stream;
    stream << "bdg-" << nextBudgetId_++;
    return stream.str();
}

}  // namespace finsight::core::services
