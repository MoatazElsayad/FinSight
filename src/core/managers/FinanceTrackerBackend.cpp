#include "FinanceTrackerBackend.h"

namespace finsight::core::managers {

services::AIService& FinanceTrackerBackend::ai() {
    return aiService_;
}

services::AuthService& FinanceTrackerBackend::auth() {
    return authService_;
}

services::TransactionService& FinanceTrackerBackend::transactions() {
    return transactionService_;
}

services::BudgetService& FinanceTrackerBackend::budgets() {
    return budgetService_;
}

services::SavingsService& FinanceTrackerBackend::savings() {
    return savingsService_;
}

services::GoalService& FinanceTrackerBackend::goals() {
    return goalService_;
}

services::AnalyticsService& FinanceTrackerBackend::analytics() {
    return analyticsService_;
}

services::SessionService& FinanceTrackerBackend::sessions() {
    return sessionService_;
}

services::ReceiptService& FinanceTrackerBackend::receipts() {
    return receiptService_;
}

services::ShoppingService& FinanceTrackerBackend::shopping() {
    return shoppingService_;
}

services::ReportService& FinanceTrackerBackend::reports() {
    return reportService_;
}

const services::AIService& FinanceTrackerBackend::ai() const {
    return aiService_;
}

const services::AuthService& FinanceTrackerBackend::auth() const {
    return authService_;
}

const services::TransactionService& FinanceTrackerBackend::transactions() const {
    return transactionService_;
}

const services::BudgetService& FinanceTrackerBackend::budgets() const {
    return budgetService_;
}

const services::SavingsService& FinanceTrackerBackend::savings() const {
    return savingsService_;
}

const services::GoalService& FinanceTrackerBackend::goals() const {
    return goalService_;
}

const services::AnalyticsService& FinanceTrackerBackend::analytics() const {
    return analyticsService_;
}

const services::SessionService& FinanceTrackerBackend::sessions() const {
    return sessionService_;
}

const services::ReceiptService& FinanceTrackerBackend::receipts() const {
    return receiptService_;
}

const services::ShoppingService& FinanceTrackerBackend::shopping() const {
    return shoppingService_;
}

const services::ReportService& FinanceTrackerBackend::reports() const {
    return reportService_;
}

}  // namespace finsight::core::managers
