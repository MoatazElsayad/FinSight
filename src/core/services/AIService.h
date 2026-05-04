#pragma once

#include "../models/AI.h"

#include <functional>
#include <string>

using namespace std;

namespace finsight::core::models {
struct FinancialReport;
}

namespace finsight::core::services {

class AnalyticsService;
class BudgetService;
class GoalService;
class ReceiptService;
class SavingsService;
class TransactionService;

class AIService {
public:
    // Builds the AI service with default provider settings.
    AIService();

    // Replaces the current provider configuration.
    void configure(models::AIProviderConfig config);
    // Returns the current provider configuration.
    const models::AIProviderConfig& config() const;

    // Generates a dashboard summary and recommendations from finance data.
    // Optional onProgress mirrors SSE-style events: trying_model, model_failed, error (detail carries reason/message).
    models::AIDashboardInsight generateDashboardInsight(const string& userId,
                                                        const models::YearMonth& period,
                                                        const AnalyticsService& analyticsService,
                                                        const TransactionService& transactionService,
                                                        const BudgetService& budgetService,
                                                        const SavingsService& savingsService,
                                                        const GoalService& goalService,
                                                        const function<void(const string& event,
                                                                            const string& model,
                                                                            const string& detail)>& onProgress =
                                                            {}) const;
    // Generates AI feedback focused on savings progress.
    models::AISavingsInsight analyzeSavings(const string& userId,
                                            const models::YearMonth& period,
                                            const SavingsService& savingsService) const;
    // Answers a finance question using the user's existing data.
    models::AIFinanceChatAnswer answerFinanceQuestion(const string& userId,
                                                      const string& question,
                                                      const models::YearMonth& period,
                                                      const AnalyticsService& analyticsService,
                                                      const TransactionService& transactionService,
                                                      const BudgetService& budgetService,
                                                      const SavingsService& savingsService,
                                                      const GoalService& goalService) const;
    // Suggests receipt details from raw text before confirmation.
    models::AIReceiptSuggestion suggestReceiptTransaction(const string& rawText,
                                                          const string& merchantHint = "") const;

    // Next-step bullets for a text export (LFM with 10s HTTP timeout; heuristic fallback if AI unavailable).
    string generateFinancialReportRecommendations(const models::FinancialReport& report) const;

private:
    // Tries the configured model list until one returns a usable response.
    models::AIChatResponse runChatCompletion(const vector<models::AIMessage>& messages,
                                            const function<void(const string& event,
                                                                const string& model,
                                                                const string& detail)>& onProgress = {}) const;
    // Builds the ordered list of models to try for a request.
    static vector<string> configuredModels(const models::AIProviderConfig& config);
    // Breaks an AI response into short bullet-like lines.
    static vector<string> splitBulletLines(const string& text);
    // Formats dashboard data into a prompt-ready context block.
    static string buildDashboardContext(const string& userId,
                                             const models::YearMonth& period,
                                             const AnalyticsService& analyticsService,
                                             const TransactionService& transactionService,
                                             const BudgetService& budgetService,
                                             const SavingsService& savingsService,
                                             const GoalService& goalService);
    // Formats savings data into a prompt-ready context block.
    static string buildSavingsContext(const string& userId,
                                           const models::YearMonth& period,
                                           const SavingsService& savingsService);
    // Picks a simple fallback merchant/category hint from receipt text.
    static string fallbackReceiptCategoryName(const string& rawText);
    static string buildFinancialReportContext(const models::FinancialReport& report);
    static string heuristicReportRecommendations(const models::FinancialReport& report);
    static string normalizeReportRecommendationLines(const string& raw);

    models::AIProviderConfig config_;
};

}  // namespace finsight::core::services
