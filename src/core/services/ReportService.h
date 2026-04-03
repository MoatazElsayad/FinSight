#pragma once

#include "../models/Report.h"

namespace finsight::core::services {

class BudgetService;
class TransactionService;

class ReportService {
public:
    models::FinancialReport generateReport(const models::ReportRequest& request,
                                           const TransactionService& transactionService,
                                           const BudgetService& budgetService) const;
};

}  // namespace finsight::core::services
