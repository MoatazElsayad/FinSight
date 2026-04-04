#include "AnalyticsService.h"

#include "BudgetService.h"
#include "GoalService.h"
#include "SavingsService.h"
#include "TransactionService.h"

#include <algorithm>
#include <unordered_map>

using namespace std;

namespace finsight::core::services {

// Builds the combined dashboard data from the finance services.
DashboardInsights AnalyticsService::buildDashboard(const std::string& userId,
                                                   const models::YearMonth& period,
                                                   const TransactionService& transactionService,
                                                   const BudgetService& budgetService,
                                                   const SavingsService& savingsService,
                                                   const GoalService& goalService) const {
    DashboardInsights insights {};
    insights.overview.income =
        transactionService.sumTransactions(userId, models::TransactionType::Income, period);
    insights.overview.expenses =
        transactionService.sumTransactions(userId, models::TransactionType::Expense, period);
    insights.overview.netSavings = insights.overview.income - insights.overview.expenses;
    insights.overview.savingsRate = insights.overview.income <= 0.0
                                        ? 0.0
                                        : insights.overview.netSavings / insights.overview.income;

    std::unordered_map<std::string, double> categoryTotals;
    for (const auto& transaction : transactionService.listTransactions(userId)) {
        if (transaction.type == models::TransactionType::Expense &&
            models::inMonth(transaction.date, period)) {
            categoryTotals[transaction.categoryId] += transaction.amount;
        }
    }

    for (const auto& [categoryId, amount] : categoryTotals) {
        const auto& category = transactionService.requireCategory(categoryId);
        insights.topExpenseCategories.push_back(CategorySpend {
            .categoryId = categoryId,
            .categoryName = category.name,
            .amount = amount,
        });
    }
    std::sort(insights.topExpenseCategories.begin(),
              insights.topExpenseCategories.end(),
              [](const auto& left, const auto& right) {
                  return left.amount > right.amount;
              });

    insights.budgetHealth = budgetService.summarizeBudgets(userId, period, transactionService);
    insights.savings = savingsService.summarize(userId, period);
    insights.investments = savingsService.investmentSnapshots(userId);

    for (const auto& goal : goalService.listGoals(userId)) {
        if (!goal.completed) {
            insights.activeGoals.push_back(goal);
        }
    }
    return insights;
}

}  // namespace finsight::core::services
