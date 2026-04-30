#include "BudgetService.h"

#include "TransactionService.h"

#include <algorithm>
#include <map>
#include <regex>
#include <sstream>
#include <stdexcept>

using namespace std;

namespace finsight::core::services {

namespace {

void validatePeriod(const models::YearMonth& period) {
    if (period.month < 1 || period.month > 12) {
        throw std::invalid_argument("Budget period month must be between 1 and 12.");
    }
}

void validateBudgetInput(const std::string& userId,
                         const std::string& categoryId,
                         const models::YearMonth& period,
                         double limit) {
    if (userId.empty()) {
        throw std::invalid_argument("Budget requires a user.");
    }
    if (categoryId.empty()) {
        throw std::invalid_argument("Budget requires a category.");
    }
    validatePeriod(period);
    if (limit <= 0.0) {
        throw std::invalid_argument("Budget limit must be greater than 0.");
    }
}

bool sameBudgetKey(const models::Budget& budget,
                   const std::string& userId,
                   const std::string& categoryId,
                   const models::YearMonth& period) {
    return budget.userId == userId && budget.categoryId == categoryId && budget.period == period;
}

}  // namespace

// Creates and stores a new monthly budget.
models::Budget BudgetService::createBudget(const std::string& userId,
                                           const std::string& categoryId,
                                           const models::YearMonth& period,
                                           double limit) {
    validateBudgetInput(userId, categoryId, period, limit);
    if (findBudget(userId, categoryId, period).has_value()) {
        throw std::invalid_argument("A budget already exists for this category and month.");
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

// Creates or replaces the user's category budget for one month.
models::Budget BudgetService::setBudget(const std::string& userId,
                                        const std::string& categoryId,
                                        const models::YearMonth& period,
                                        double limit) {
    validateBudgetInput(userId, categoryId, period, limit);
    for (auto& budget : budgets_) {
        if (sameBudgetKey(budget, userId, categoryId, period)) {
            budget.limit = limit;
            return budget;
        }
    }
    return createBudget(userId, categoryId, period, limit);
}

// Updates one stored budget.
models::Budget BudgetService::updateBudget(const std::string& userId,
                                           const std::string& budgetId,
                                           const models::Budget& updatedBudget) {
    validateBudgetInput(userId, updatedBudget.categoryId, updatedBudget.period, updatedBudget.limit);
    for (const auto& budget : budgets_) {
        if (budget.id != budgetId &&
            sameBudgetKey(budget, userId, updatedBudget.categoryId, updatedBudget.period)) {
            throw std::invalid_argument("Another budget already exists for this category and month.");
        }
    }
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
    if (userId.empty()) {
        throw std::invalid_argument("Budget copy requires a user.");
    }
    validatePeriod(from);
    validatePeriod(to);
    if (from == to) {
        throw std::invalid_argument("Source and target budget months must be different.");
    }
    const auto sourceBudgets = listBudgets(userId, from);
    for (const auto& budget : sourceBudgets) {
        setBudget(userId, budget.categoryId, to, budget.limit);
    }
}

// Finds the user's budget for one category/month.
std::optional<models::Budget> BudgetService::findBudget(const std::string& userId,
                                                        const std::string& categoryId,
                                                        const models::YearMonth& period) const {
    for (const auto& budget : budgets_) {
        if (sameBudgetKey(budget, userId, categoryId, period)) {
            return budget;
        }
    }
    return std::nullopt;
}

// Returns all budgets for the selected user and month.
std::vector<models::Budget> BudgetService::listBudgets(const std::string& userId,
                                                       const models::YearMonth& period) const {
    validatePeriod(period);
    std::vector<models::Budget> result;
    for (const auto& budget : budgets_) {
        if (budget.userId == userId && budget.period == period) {
            result.push_back(budget);
        }
    }
    std::sort(result.begin(), result.end(), [](const auto& left, const auto& right) {
        return left.categoryId < right.categoryId;
    });
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

// Builds a month-over-month budget comparison for UI and reports.
models::BudgetPeriodComparison BudgetService::comparePeriods(
    const std::string& userId,
    const models::YearMonth& current,
    const models::YearMonth& previous,
    const TransactionService& transactionService) const {
    validatePeriod(current);
    validatePeriod(previous);

    models::BudgetPeriodComparison comparison {
        .currentPeriod = current,
        .previousPeriod = previous,
    };

    std::map<std::string, models::BudgetComparisonLine> lines;

    auto mergeStatus = [&](const models::BudgetStatus& status, bool isCurrent) {
        const auto& category = transactionService.requireCategory(status.budget.categoryId);
        auto& line = lines[status.budget.categoryId];
        line.categoryId = status.budget.categoryId;
        line.categoryName = category.name;
        if (isCurrent) {
            line.currentLimit += status.budget.limit;
            line.currentSpent += status.spent;
            comparison.currentTotalLimit += status.budget.limit;
            comparison.currentTotalSpent += status.spent;
        } else {
            line.previousLimit += status.budget.limit;
            line.previousSpent += status.spent;
            comparison.previousTotalLimit += status.budget.limit;
            comparison.previousTotalSpent += status.spent;
        }
    };

    for (const auto& status : summarizeBudgets(userId, current, transactionService)) {
        mergeStatus(status, true);
    }
    for (const auto& status : summarizeBudgets(userId, previous, transactionService)) {
        mergeStatus(status, false);
    }

    comparison.totalLimitDelta = comparison.currentTotalLimit - comparison.previousTotalLimit;
    comparison.totalSpentDelta = comparison.currentTotalSpent - comparison.previousTotalSpent;

    for (auto& [_, line] : lines) {
        line.limitDelta = line.currentLimit - line.previousLimit;
        line.spentDelta = line.currentSpent - line.previousSpent;
        comparison.categories.push_back(line);
    }

    std::sort(comparison.categories.begin(),
              comparison.categories.end(),
              [](const auto& left, const auto& right) {
                  return left.categoryName < right.categoryName;
              });

    return comparison;
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
