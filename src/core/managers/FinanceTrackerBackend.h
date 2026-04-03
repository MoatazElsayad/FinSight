#pragma once

#include "../services/AIService.h"
#include "../services/AnalyticsService.h"
#include "../services/AuthService.h"
#include "../services/BudgetService.h"
#include "../services/GoalService.h"
#include "../services/ReceiptService.h"
#include "../services/ReportService.h"
#include "../services/SavingsService.h"
#include "../services/SessionService.h"
#include "../services/ShoppingService.h"
#include "../services/TransactionService.h"

namespace finsight::core::managers {

class FinanceTrackerBackend {
public:
    services::AIService& ai();
    services::AuthService& auth();
    services::TransactionService& transactions();
    services::BudgetService& budgets();
    services::SavingsService& savings();
    services::GoalService& goals();
    services::AnalyticsService& analytics();
    services::SessionService& sessions();
    services::ReceiptService& receipts();
    services::ShoppingService& shopping();
    services::ReportService& reports();

    const services::AIService& ai() const;
    const services::AuthService& auth() const;
    const services::TransactionService& transactions() const;
    const services::BudgetService& budgets() const;
    const services::SavingsService& savings() const;
    const services::GoalService& goals() const;
    const services::AnalyticsService& analytics() const;
    const services::SessionService& sessions() const;
    const services::ReceiptService& receipts() const;
    const services::ShoppingService& shopping() const;
    const services::ReportService& reports() const;

private:
    services::AIService aiService_;
    services::AuthService authService_;
    services::TransactionService transactionService_;
    services::BudgetService budgetService_;
    services::SavingsService savingsService_;
    services::GoalService goalService_;
    services::AnalyticsService analyticsService_;
    services::SessionService sessionService_;
    services::ReceiptService receiptService_;
    services::ShoppingService shoppingService_;
    services::ReportService reportService_;
};

}  // namespace finsight::core::managers
