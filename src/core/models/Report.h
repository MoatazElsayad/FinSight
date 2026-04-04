#pragma once

#include "Budget.h"
#include "Common.h"
#include "Transaction.h"

#include <string>
#include <vector>

using namespace std;

namespace finsight::core::models {

// Describes the requested range for a financial report.
struct ReportRequest {
    std::string userId;
    Date from;
    Date to;
};

// Stores category totals inside a generated report.
struct CategoryReportLine {
    std::string categoryId;
    std::string categoryName;
    double amount {0.0};
};

// Stores the full output of a generated report.
struct FinancialReport {
    ReportRequest request;
    double totalIncome {0.0};
    double totalExpenses {0.0};
    double net {0.0};
    std::vector<Transaction> transactions;
    std::vector<CategoryReportLine> categoryExpenses;
    std::vector<BudgetStatus> impactedBudgets;
    std::string exportedText;
};

}  // namespace finsight::core::models
