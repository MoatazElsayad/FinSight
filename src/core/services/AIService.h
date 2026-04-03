#pragma once

#include "../models/AI.h"

namespace finsight::core::services {

class AnalyticsService;
class BudgetService;
class GoalService;
class ReceiptService;
class SavingsService;
class TransactionService;

class AIService {
public:
    AIService();

    void configure(models::AIProviderConfig config);
    const models::AIProviderConfig& config() const;

    models::AIDashboardInsight generateDashboardInsight(const std::string& userId,
                                                        const models::YearMonth& period,
                                                        const AnalyticsService& analyticsService,
                                                        const TransactionService& transactionService,
                                                        const BudgetService& budgetService,
                                                        const SavingsService& savingsService,
                                                        const GoalService& goalService) const;
    models::AISavingsInsight analyzeSavings(const std::string& userId,
                                            const models::YearMonth& period,
                                            const SavingsService& savingsService) const;
    models::AIFinanceChatAnswer answerFinanceQuestion(const std::string& userId,
                                                      const std::string& question,
                                                      const models::YearMonth& period,
                                                      const AnalyticsService& analyticsService,
                                                      const TransactionService& transactionService,
                                                      const BudgetService& budgetService,
                                                      const SavingsService& savingsService,
                                                      const GoalService& goalService) const;
    models::AIReceiptSuggestion suggestReceiptTransaction(const std::string& rawText,
                                                          const std::string& merchantHint = "") const;

private:
    models::AIChatResponse runChatCompletion(const std::vector<models::AIMessage>& messages) const;
    static std::vector<std::string> configuredModels(const models::AIProviderConfig& config);
    static std::vector<std::string> splitBulletLines(const std::string& text);
    static std::string buildDashboardContext(const std::string& userId,
                                             const models::YearMonth& period,
                                             const AnalyticsService& analyticsService,
                                             const TransactionService& transactionService,
                                             const BudgetService& budgetService,
                                             const SavingsService& savingsService,
                                             const GoalService& goalService);
    static std::string buildSavingsContext(const std::string& userId,
                                           const models::YearMonth& period,
                                           const SavingsService& savingsService);
    static std::string fallbackReceiptCategoryName(const std::string& rawText);

    models::AIProviderConfig config_;
};

}  // namespace finsight::core::services
