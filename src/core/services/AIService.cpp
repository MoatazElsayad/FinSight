#include "AIService.h"

#include "AnalyticsService.h"
#include "BudgetService.h"
#include "GoalService.h"
#include "SavingsService.h"
#include "TransactionService.h"
#include "../../network/ai/ChatCompletionClient.h"

#include <algorithm>
#include <cctype>
#include <sstream>

namespace finsight::core::services {

AIService::AIService() = default;

void AIService::configure(models::AIProviderConfig config) {
    config_ = std::move(config);
}

const models::AIProviderConfig& AIService::config() const {
    return config_;
}

models::AIDashboardInsight AIService::generateDashboardInsight(const std::string& userId,
                                                               const models::YearMonth& period,
                                                               const AnalyticsService& analyticsService,
                                                               const TransactionService& transactionService,
                                                               const BudgetService& budgetService,
                                                               const SavingsService& savingsService,
                                                               const GoalService& goalService) const {
    const auto context = buildDashboardContext(userId,
                                               period,
                                               analyticsService,
                                               transactionService,
                                               budgetService,
                                               savingsService,
                                               goalService);
    const auto response = runChatCompletion({
        {"system", "You are a personal finance assistant. Respond briefly and focus on concrete actions."},
        {"user", "Summarize this finance dashboard and give 3 practical recommendations.\n\n" + context},
    });

    models::AIDashboardInsight insight;
    insight.summary = response.content;
    insight.recommendations = splitBulletLines(response.content);
    insight.usedFallback = response.usedFallback;
    return insight;
}

models::AISavingsInsight AIService::analyzeSavings(const std::string& userId,
                                                   const models::YearMonth& period,
                                                   const SavingsService& savingsService) const {
    const auto context = buildSavingsContext(userId, period, savingsService);
    const auto response = runChatCompletion({
        {"system", "You are a savings coach. Respond with short, direct, encouraging advice."},
        {"user", "Analyze the user's savings situation and suggest 3 actions.\n\n" + context},
    });

    models::AISavingsInsight insight;
    insight.summary = response.content;
    insight.actions = splitBulletLines(response.content);
    insight.usedFallback = response.usedFallback;
    return insight;
}

models::AIFinanceChatAnswer AIService::answerFinanceQuestion(const std::string& userId,
                                                             const std::string& question,
                                                             const models::YearMonth& period,
                                                             const AnalyticsService& analyticsService,
                                                             const TransactionService& transactionService,
                                                             const BudgetService& budgetService,
                                                             const SavingsService& savingsService,
                                                             const GoalService& goalService) const {
    const auto context = buildDashboardContext(userId,
                                               period,
                                               analyticsService,
                                               transactionService,
                                               budgetService,
                                               savingsService,
                                               goalService);
    const auto response = runChatCompletion({
        {"system", "You are a finance assistant for a personal finance tracker. Answer based only on the supplied data."},
        {"user", "Finance data:\n" + context + "\n\nQuestion: " + question},
    });

    return models::AIFinanceChatAnswer {
        .answer = response.content,
        .usedFallback = response.usedFallback,
    };
}

models::AIReceiptSuggestion AIService::suggestReceiptTransaction(const std::string& rawText,
                                                                 const std::string& merchantHint) const {
    const auto response = runChatCompletion({
        {"system", "You extract transaction details from receipt text. Keep the response short and structured."},
        {"user", "Infer merchant, amount, likely category, and confidence notes from this receipt text:\n" +
                     rawText +
                     "\nMerchant hint: " + merchantHint},
    });

    models::AIReceiptSuggestion suggestion;
    suggestion.parseResult.receiptId = "";
    suggestion.parseResult.merchant = merchantHint.empty() ? fallbackReceiptCategoryName(rawText) : merchantHint;
    suggestion.parseResult.confidenceNotes = response.content;
    suggestion.parseResult.suggestedCategoryId = "";
    suggestion.usedFallback = response.usedFallback;
    return suggestion;
}

models::AIChatResponse AIService::runChatCompletion(const std::vector<models::AIMessage>& messages) const {
    network::ai::ChatCompletionClient client;
    models::AIChatResponse lastResponse;
    const auto models = configuredModels(config_);

    for (std::size_t index = 0; index < models.size(); ++index) {
        auto response = client.complete(config_, models::AIChatRequest {
                                                     .model = models[index],
                                                     .messages = messages,
                                                     .temperature = 0.2,
                                                 });
        response.attemptedModels = models;
        response.usedFallback = response.usedFallback || index > 0;
        if (response.success) {
            return response;
        }
        lastResponse = std::move(response);
    }

    if (lastResponse.content.empty()) {
        lastResponse.content = "All configured AI models failed to return a usable response.";
        lastResponse.error = "No configured model returned a successful chat completion.";
    }
    lastResponse.attemptedModels = models;
    lastResponse.usedFallback = true;
    return lastResponse;
}

std::vector<std::string> AIService::configuredModels(const models::AIProviderConfig& config) {
    std::vector<std::string> models;
    if (!config.model.empty()) {
        models.push_back(config.model);
    }
    for (const auto& fallback : config.fallbackModels) {
        if (!fallback.empty() &&
            std::find(models.begin(), models.end(), fallback) == models.end()) {
            models.push_back(fallback);
        }
    }
    if (models.empty()) {
        models.push_back("openrouter/free");
    }
    return models;
}

std::vector<std::string> AIService::splitBulletLines(const std::string& text) {
    std::vector<std::string> lines;
    std::istringstream stream(text);
    std::string line;
    while (std::getline(stream, line)) {
        line.erase(line.begin(),
                   std::find_if(line.begin(), line.end(), [](unsigned char ch) {
                       return !std::isspace(ch) && ch != '-' && ch != '*' && ch != '1' && ch != '.' && ch != ')';
                   }));
        if (!line.empty()) {
            lines.push_back(line);
        }
    }
    return lines;
}

std::string AIService::buildDashboardContext(const std::string& userId,
                                             const models::YearMonth& period,
                                             const AnalyticsService& analyticsService,
                                             const TransactionService& transactionService,
                                             const BudgetService& budgetService,
                                             const SavingsService& savingsService,
                                             const GoalService& goalService) {
    const auto dashboard = analyticsService.buildDashboard(
        userId,
        period,
        transactionService,
        budgetService,
        savingsService,
        goalService);

    std::ostringstream context;
    context << "Period: " << period.year << "-" << period.month << "\n";
    context << "Income: " << dashboard.overview.income << "\n";
    context << "Expenses: " << dashboard.overview.expenses << "\n";
    context << "Net savings: " << dashboard.overview.netSavings << "\n";
    context << "Savings rate: " << dashboard.overview.savingsRate << "\n";
    context << "Savings balance: " << dashboard.savings.currentBalance << "\n";
    context << "Monthly savings target: " << dashboard.savings.monthlyTarget << "\n";
    context << "Top categories:\n";
    for (const auto& category : dashboard.topExpenseCategories) {
        context << "- " << category.categoryName << ": " << category.amount << "\n";
    }
    context << "Budgets:\n";
    for (const auto& budget : dashboard.budgetHealth) {
        context << "- categoryId=" << budget.budget.categoryId
                << ", spent=" << budget.spent
                << ", limit=" << budget.budget.limit
                << ", remaining=" << budget.remaining
                << ", overspent=" << (budget.overspent ? "yes" : "no") << "\n";
    }
    context << "Active goals: " << dashboard.activeGoals.size() << "\n";
    return context.str();
}

std::string AIService::buildSavingsContext(const std::string& userId,
                                           const models::YearMonth& period,
                                           const SavingsService& savingsService) {
    const auto savings = savingsService.summarize(userId, period);
    std::ostringstream context;
    context << "Period: " << period.year << "-" << period.month << "\n";
    context << "Current balance: " << savings.currentBalance << "\n";
    context << "Monthly saved: " << savings.monthlySaved << "\n";
    context << "Monthly target: " << savings.monthlyTarget << "\n";
    context << "Progress ratio: " << savings.progressToMonthlyTarget << "\n";
    context << "Long-term target: " << savings.longTermTarget << "\n";
    return context.str();
}

std::string AIService::fallbackReceiptCategoryName(const std::string& rawText) {
    if (models::containsCaseInsensitive(rawText, "market") ||
        models::containsCaseInsensitive(rawText, "grocery") ||
        models::containsCaseInsensitive(rawText, "food")) {
        return "Food";
    }
    if (models::containsCaseInsensitive(rawText, "fuel") ||
        models::containsCaseInsensitive(rawText, "uber") ||
        models::containsCaseInsensitive(rawText, "transport")) {
        return "Transport";
    }
    return "Receipt Merchant";
}

}  // namespace finsight::core::services
