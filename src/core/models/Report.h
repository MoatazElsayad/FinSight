#pragma once

#include "Budget.h"
#include "Common.h"
#include "Transaction.h"

#include <string>
#include <vector>

namespace finsight::core::models {

struct ReportRequest {
    std::string userId;
    Date from;
    Date to;
};

struct CategoryReportLine {
    std::string categoryId;
    std::string categoryName;
    double amount {0.0};
};

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
