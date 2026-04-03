#pragma once

#include "../models/Budget.h"

#include <cstddef>
#include <string>
#include <vector>

namespace finsight::core::services {

class TransactionService;

class BudgetService {
public:
    models::Budget createBudget(const std::string& userId,
                                const std::string& categoryId,
                                const models::YearMonth& period,
                                double limit);
    models::Budget updateBudget(const std::string& userId,
                                const std::string& budgetId,
                                const models::Budget& updatedBudget);
    void deleteBudget(const std::string& userId, const std::string& budgetId);
    void copyBudgets(const std::string& userId,
                     const models::YearMonth& from,
                     const models::YearMonth& to);

    std::vector<models::Budget> listBudgets(const std::string& userId,
                                            const models::YearMonth& period) const;
    std::vector<models::BudgetStatus> summarizeBudgets(const std::string& userId,
                                                       const models::YearMonth& period,
                                                       const TransactionService& transactionService) const;
    std::vector<models::Budget> allBudgets() const;
    void loadBudgets(std::vector<models::Budget> budgets);

private:
    std::string nextId();

    std::vector<models::Budget> budgets_;
    std::size_t nextBudgetId_ {1};
};

}  // namespace finsight::core::services
