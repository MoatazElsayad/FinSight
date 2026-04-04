#pragma once

#include "../models/Budget.h"
#include "../models/Goal.h"
#include "../models/Investment.h"
#include "../models/Savings.h"

#include <string>
#include <vector>

using namespace std;

namespace finsight::core::services {

class TransactionService;
class BudgetService;
class SavingsService;
class GoalService;

// Stores total spending for one expense category.
struct CategorySpend {
    std::string categoryId;
    std::string categoryName;
    double amount {0.0};
};

// Stores the key monthly finance totals.
struct MonthlyOverview {
    double income {0.0};
    double expenses {0.0};
    double netSavings {0.0};
    double savingsRate {0.0};
};

// Stores the combined dashboard data used by the app.
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
    // Builds the dashboard state from the current backend services.
    DashboardInsights buildDashboard(const std::string& userId,
                                     const models::YearMonth& period,
                                     const TransactionService& transactionService,
                                     const BudgetService& budgetService,
                                     const SavingsService& savingsService,
                                     const GoalService& goalService) const;
};

}  // namespace finsight::core::services
