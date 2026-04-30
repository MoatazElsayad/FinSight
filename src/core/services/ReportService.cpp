#include "ReportService.h"

#include "BudgetService.h"
#include "TransactionService.h"

#include <algorithm>
#include <iomanip>
#include <map>
#include <sstream>
#include <stdexcept>

using namespace std;

namespace finsight::core::services {

// Generates a report by filtering transactions and summarizing the results.
models::FinancialReport ReportService::generateReport(const models::ReportRequest& request,
                                                      const TransactionService& transactionService,
                                                      const BudgetService& budgetService) const {
    if (request.userId.empty()) {
        throw std::invalid_argument("Report requires a user.");
    }
    if (request.to < request.from) {
        throw std::invalid_argument("Report end date cannot be before start date.");
    }

    models::TransactionFilter filter;
    filter.from = request.from;
    filter.to = request.to;

    models::FinancialReport report {.request = request};
    report.transactions = transactionService.filterTransactions(request.userId, filter);

    std::map<std::string, models::CategoryReportLine> categoryTotals;
    for (const auto& transaction : report.transactions) {
        if (transaction.type == models::TransactionType::Income) {
            report.totalIncome += transaction.amount;
        } else {
            report.totalExpenses += transaction.amount;
            const auto& category = transactionService.requireCategory(transaction.categoryId);
            auto& line = categoryTotals[category.id];
            line.categoryId = category.id;
            line.categoryName = category.name;
            line.amount += transaction.amount;
        }
    }
    report.net = report.totalIncome - report.totalExpenses;

    for (const auto& [_, line] : categoryTotals) {
        report.categoryExpenses.push_back(line);
    }
    std::sort(report.categoryExpenses.begin(),
              report.categoryExpenses.end(),
              [](const auto& left, const auto& right) {
                  return left.amount > right.amount;
              });

    models::YearMonth currentPeriod {request.from.year, request.from.month};
    report.impactedBudgets = budgetService.summarizeBudgets(request.userId, currentPeriod, transactionService);

    std::ostringstream text;
    text << "FinSight Report\n";
    text << "Range: " << request.from.toString() << " to " << request.to.toString() << "\n";
    text << std::fixed << std::setprecision(2);
    text << "Income: " << report.totalIncome << "\n";
    text << "Expenses: " << report.totalExpenses << "\n";
    text << "Net: " << report.net << "\n";
    text << "Transactions: " << report.transactions.size() << "\n";
    text << "\nCategory Expenses\n";
    if (report.categoryExpenses.empty()) {
        text << "- None\n";
    } else {
        for (const auto& line : report.categoryExpenses) {
            text << "- " << line.categoryName << ": " << line.amount << "\n";
        }
    }
    text << "\nBudget Impact\n";
    if (report.impactedBudgets.empty()) {
        text << "- No budgets configured for " << currentPeriod.year << '-'
             << std::setw(2) << std::setfill('0') << currentPeriod.month << "\n";
        text << std::setfill(' ');
    } else {
        for (const auto& status : report.impactedBudgets) {
            const auto& category = transactionService.requireCategory(status.budget.categoryId);
            text << "- " << category.name
                 << ": limit " << status.budget.limit
                 << ", spent " << status.spent
                 << ", remaining " << status.remaining
                 << (status.overspent ? " (overspent)" : "")
                 << "\n";
        }
    }
    text << "\nTransactions\n";
    if (report.transactions.empty()) {
        text << "- None\n";
    } else {
        for (const auto& transaction : report.transactions) {
            const auto& category = transactionService.requireCategory(transaction.categoryId);
            text << "- " << transaction.date.toString()
                 << " | " << (transaction.type == models::TransactionType::Income ? "Income" : "Expense")
                 << " | " << category.name
                 << " | " << transaction.title
                 << " | " << transaction.amount << "\n";
        }
    }
    report.exportedText = text.str();

    return report;
}

}  // namespace finsight::core::services
