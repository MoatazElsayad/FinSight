#pragma once

#include "../models/Budget.h"
#include "../models/Goal.h"
#include "../models/Investment.h"
#include "../models/Savings.h"

#include <string>
#include <vector>

namespace finsight::core::services {

class TransactionService;
class BudgetService;
class SavingsService;
class GoalService;

struct CategorySpend {
    std::string categoryId;
    std::string categoryName;
    double amount {0.0};
};

struct MonthlyOverview {
    double income {0.0};
    double expenses {0.0};
    double netSavings {0.0};
    double savingsRate {0.0};
};

struct DashboardInsights {
    MonthlyOverview overview;
    std::vector<CategorySpend> topExpenseCategories;
    std::vector<models::BudgetStatus> budgetHealth;
    models::SavingsOverview savings;
    std::vector<models::InvestmentSnapshot> investments;
    std::vector<models::Goal> activeGoals;
};

class AnalyticsService {
public:
    DashboardInsights buildDashboard(const std::string& userId,
                                     const models::YearMonth& period,
                                     const TransactionService& transactionService,
                                     const BudgetService& budgetService,
                                     const SavingsService& savingsService,
                                     const GoalService& goalService) const;
};

}  // namespace finsight::core::services
