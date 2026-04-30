#pragma once

#include "../models/Budget.h"

#include <cstddef>
#include <optional>
#include <string>
#include <vector>

using namespace std;

namespace finsight::core::services {

class TransactionService;

class BudgetService {
public:
    // Creates a new monthly budget for a category.
    models::Budget createBudget(const std::string& userId,
                                const std::string& categoryId,
                                const models::YearMonth& period,
                                double limit);
    // Creates a budget or updates the existing budget for the same category/month.
    models::Budget setBudget(const std::string& userId,
                             const std::string& categoryId,
                             const models::YearMonth& period,
                             double limit);
    // Updates an existing budget.
    models::Budget updateBudget(const std::string& userId,
                                const std::string& budgetId,
                                const models::Budget& updatedBudget);
    // Deletes one budget by id.
    void deleteBudget(const std::string& userId, const std::string& budgetId);
    // Copies one month's budgets into another month.
    void copyBudgets(const std::string& userId,
                     const models::YearMonth& from,
                     const models::YearMonth& to);
    // Returns a matching budget for a user/category/month if one exists.
    std::optional<models::Budget> findBudget(const std::string& userId,
                                             const std::string& categoryId,
                                             const models::YearMonth& period) const;

    // Returns all budgets for a user in a given month.
    std::vector<models::Budget> listBudgets(const std::string& userId,
                                            const models::YearMonth& period) const;
    // Builds spending summaries for each budget in a month.
    std::vector<models::BudgetStatus> summarizeBudgets(const std::string& userId,
                                                       const models::YearMonth& period,
                                                       const TransactionService& transactionService) const;
    // Compares budget limits and actual spending between two months.
    models::BudgetPeriodComparison comparePeriods(const std::string& userId,
                                                  const models::YearMonth& current,
                                                  const models::YearMonth& previous,
                                                  const TransactionService& transactionService) const;
    // Returns every stored budget.
    std::vector<models::Budget> allBudgets() const;
    // Replaces the stored budgets from persisted data.
    void loadBudgets(std::vector<models::Budget> budgets);

private:
    // Generates the next budget id.
    std::string nextId();

    std::vector<models::Budget> budgets_;
    std::size_t nextBudgetId_ {1};
};

}  // namespace finsight::core::services
