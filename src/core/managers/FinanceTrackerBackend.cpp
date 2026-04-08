#include "FinanceTrackerBackend.h"

using namespace std;

namespace finsight::core::managers {

// Returns the AI service instance.
services::AIService& FinanceTrackerBackend::ai() {
    return aiService_;
}

// Returns the auth service instance.
services::AuthService& FinanceTrackerBackend::auth() {
    return authService_;
}

// Returns the transaction service instance.
services::TransactionService& FinanceTrackerBackend::transactions() {
    return transactionService_;
}

// Returns the budget service instance.
services::BudgetService& FinanceTrackerBackend::budgets() {
    return budgetService_;
}

// Returns the budget alert service instance.
services::BudgetAlertService& FinanceTrackerBackend::budgetAlerts() {
    return budgetAlertService_;
}

// Returns the email service instance.
services::EmailService& FinanceTrackerBackend::email() {
    return emailService_;
}

// Returns the savings service instance.
services::SavingsService& FinanceTrackerBackend::savings() {
    return savingsService_;
}

// Returns the goals service instance.
services::GoalService& FinanceTrackerBackend::goals() {
    return goalService_;
}

// Returns the analytics service instance.
services::AnalyticsService& FinanceTrackerBackend::analytics() {
    return analyticsService_;
}

// Returns the session service instance.
services::SessionService& FinanceTrackerBackend::sessions() {
    return sessionService_;
}

// Returns the receipt service instance.
services::ReceiptService& FinanceTrackerBackend::receipts() {
    return receiptService_;
}

// Returns the shopping service instance.
services::ShoppingService& FinanceTrackerBackend::shopping() {
    return shoppingService_;
}

// Returns the report service instance.
services::ReportService& FinanceTrackerBackend::reports() {
    return reportService_;
}

// Returns the AI service instance as const.
const services::AIService& FinanceTrackerBackend::ai() const {
    return aiService_;
}

// Returns the auth service instance as const.
const services::AuthService& FinanceTrackerBackend::auth() const {
    return authService_;
}

// Returns the transaction service instance as const.
const services::TransactionService& FinanceTrackerBackend::transactions() const {
    return transactionService_;
}

// Returns the budget service instance as const.
const services::BudgetService& FinanceTrackerBackend::budgets() const {
    return budgetService_;
}

// Returns the budget alert service instance as const.
const services::BudgetAlertService& FinanceTrackerBackend::budgetAlerts() const {
    return budgetAlertService_;
}

// Returns the email service instance as const.
const services::EmailService& FinanceTrackerBackend::email() const {
    return emailService_;
}

// Returns the savings service instance as const.
const services::SavingsService& FinanceTrackerBackend::savings() const {
    return savingsService_;
}

// Returns the goals service instance as const.
const services::GoalService& FinanceTrackerBackend::goals() const {
    return goalService_;
}

// Returns the analytics service instance as const.
const services::AnalyticsService& FinanceTrackerBackend::analytics() const {
    return analyticsService_;
}

// Returns the session service instance as const.
const services::SessionService& FinanceTrackerBackend::sessions() const {
    return sessionService_;
}

// Returns the receipt service instance as const.
const services::ReceiptService& FinanceTrackerBackend::receipts() const {
    return receiptService_;
}

// Returns the shopping service instance as const.
const services::ShoppingService& FinanceTrackerBackend::shopping() const {
    return shoppingService_;
}

// Returns the report service instance as const.
const services::ReportService& FinanceTrackerBackend::reports() const {
    return reportService_;
}

}  // namespace finsight::core::managers
