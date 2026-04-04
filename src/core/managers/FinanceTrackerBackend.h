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

using namespace std;

namespace finsight::core::managers {

class FinanceTrackerBackend {
public:
    // Exposes the AI integration service.
    services::AIService& ai();
    // Exposes the authentication service.
    services::AuthService& auth();
    // Exposes the transaction and category service.
    services::TransactionService& transactions();
    // Exposes the budget service.
    services::BudgetService& budgets();
    // Exposes the savings and investment service.
    services::SavingsService& savings();
    // Exposes the financial goals service.
    services::GoalService& goals();
    // Exposes the analytics and dashboard service.
    services::AnalyticsService& analytics();
    // Exposes the session management service.
    services::SessionService& sessions();
    // Exposes the receipt processing service.
    services::ReceiptService& receipts();
    // Exposes the pantry and shopping service.
    services::ShoppingService& shopping();
    // Exposes the reporting service.
    services::ReportService& reports();

    // Returns the AI integration service.
    const services::AIService& ai() const;
    // Returns the authentication service.
    const services::AuthService& auth() const;
    // Returns the transaction and category service.
    const services::TransactionService& transactions() const;
    // Returns the budget service.
    const services::BudgetService& budgets() const;
    // Returns the savings and investment service.
    const services::SavingsService& savings() const;
    // Returns the financial goals service.
    const services::GoalService& goals() const;
    // Returns the analytics and dashboard service.
    const services::AnalyticsService& analytics() const;
    // Returns the session management service.
    const services::SessionService& sessions() const;
    // Returns the receipt processing service.
    const services::ReceiptService& receipts() const;
    // Returns the pantry and shopping service.
    const services::ShoppingService& shopping() const;
    // Returns the reporting service.
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
