#pragma once

#include "../models/Report.h"

using namespace std;

namespace finsight::core::services {

class BudgetService;
class TransactionService;

class ReportService {
public:
    // Generates a financial report for a selected date range.
    models::FinancialReport generateReport(const models::ReportRequest& request,
                                           const TransactionService& transactionService,
                                           const BudgetService& budgetService) const;
};

}  // namespace finsight::core::services
