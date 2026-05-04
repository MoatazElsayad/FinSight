#include "AIService.h"

#include "AnalyticsService.h"
#include "BudgetService.h"
#include "GoalService.h"
#include "SavingsService.h"
#include "TransactionService.h"
#include "../models/Report.h"
#include "../../network/ai/ChatCompletionClient.h"

#include <algorithm>
#include <cctype>
#include <chrono>
#include <random>
#include <sstream>
#include <thread>

using namespace std;

namespace finsight::core::services {

namespace {

bool openRouterKeyLooksConfigured(const string& apiKey) {
    return !apiKey.empty() && apiKey.find("sk-or-v1") == 0;
}

}  // namespace

// Builds the AI service with default settings.
AIService::AIService() = default;

// Replaces the current provider configuration.
void AIService::configure(models::AIProviderConfig config) {
    config_ = move(config);
}

// Returns the current provider configuration.
const models::AIProviderConfig& AIService::config() const {
    return config_;
}

// Generates AI commentary for the dashboard view.
models::AIDashboardInsight AIService::generateDashboardInsight(const string& userId,
                                                               const models::YearMonth& period,
                                                               const AnalyticsService& analyticsService,
                                                               const TransactionService& transactionService,
                                                               const BudgetService& budgetService,
                                                               const SavingsService& savingsService,
                                                               const GoalService& goalService,
                                                               const function<void(const string& event,
                                                                                   const string& model,
                                                                                   const string& detail)>& onProgress) const {
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
    },
                                            onProgress);

    models::AIDashboardInsight insight;
    insight.summary = response.content;
    insight.recommendations = splitBulletLines(response.content);
    insight.usedFallback = response.usedFallback;
    insight.model = response.model;
    insight.allModelsBusy = !response.success;
    if (!response.success && onProgress) {
        onProgress("error",
                   "",
                   "All AI models are busy or rate-limited right now. Please try again in a few minutes.");
    }
    return insight;
}

// Generates AI commentary focused on savings progress.
models::AISavingsInsight AIService::analyzeSavings(const string& userId,
                                                   const models::YearMonth& period,
                                                   const SavingsService& savingsService) const {
    const auto context = buildSavingsContext(userId, period, savingsService);
    const auto response = runChatCompletion({
        {"system", "You are a savings coach. Respond with short, direct, encouraging advice."},
        {"user", "Analyze the user's savings situation and suggest 3 actions.\n\n" + context},
    },
                                            {});

    models::AISavingsInsight insight;
    insight.summary = response.content;
    insight.actions = splitBulletLines(response.content);
    insight.usedFallback = response.usedFallback;
    return insight;
}

// Answers a free-form finance question using the current backend data.
models::AIFinanceChatAnswer AIService::answerFinanceQuestion(const string& userId,
                                                             const string& question,
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
    },
                                            {});

    return models::AIFinanceChatAnswer {
        .answer = response.content,
        .usedFallback = response.usedFallback,
    };
}

// Asks the model for receipt interpretation guidance from raw text.
models::AIReceiptSuggestion AIService::suggestReceiptTransaction(const string& rawText,
                                                                 const string& merchantHint) const {
    const auto response = runChatCompletion({
        {"system", "You extract transaction details from receipt text. Keep the response short and structured."},
        {"user", "Infer merchant, amount, likely category, and confidence notes from this receipt text:\n" +
                     rawText +
                     "\nMerchant hint: " + merchantHint},
    },
                                            {});

    models::AIReceiptSuggestion suggestion;
    suggestion.parseResult.receiptId = "";
    suggestion.parseResult.merchant = merchantHint.empty() ? fallbackReceiptCategoryName(rawText) : merchantHint;
    suggestion.parseResult.confidenceNotes = response.content;
    suggestion.parseResult.suggestedCategoryId = "";
    suggestion.usedFallback = response.usedFallback;
    return suggestion;
}

// Tries the configured model list until one returns a valid answer.
models::AIChatResponse AIService::runChatCompletion(
    const vector<models::AIMessage>& messages,
    const function<void(const string& event, const string& model, const string& detail)>& onProgress) const {
    network::ai::ChatCompletionClient client;
    models::AIChatResponse lastResponse;
    auto models = configuredModels(config_);

    if (models.size() > 1) {
        random_device rd;
        mt19937 g(rd());
        shuffle(models.begin(), models.end(), g);
    }

    for (size_t index = 0; index < models.size(); ++index) {
        if (onProgress) {
            onProgress("trying_model", models[index], {});
        }
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
        string reason = response.error;
        if (reason.empty()) {
            reason = response.content.empty() ? "Empty response" : response.content;
        }
        if (onProgress) {
            onProgress("model_failed", models[index], reason);
        }
        if (response.httpStatus == 429) {
            this_thread::sleep_for(chrono::milliseconds(500));
        }
        lastResponse = move(response);
    }

    if (lastResponse.content.empty()) {
        lastResponse.content =
            "All AI models are busy or rate-limited right now. Please try again in a few minutes.";
        lastResponse.error = "No configured model returned a successful response.";
    }
    lastResponse.attemptedModels = models;
    lastResponse.usedFallback = true;
    lastResponse.success = false;
    return lastResponse;
}

// Builds the ordered model list used for retries.
vector<string> AIService::configuredModels(const models::AIProviderConfig& config) {
    vector<string> models;
    if (!config.model.empty()) {
        models.push_back(config.model);
    }
    for (const auto& fallback : config.fallbackModels) {
        if (!fallback.empty() &&
            find(models.begin(), models.end(), fallback) == models.end()) {
            models.push_back(fallback);
        }
    }
    if (models.empty()) {
        models.push_back("openrouter/free");
    }
    return models;
}

// Splits an AI response into short recommendation lines.
vector<string> AIService::splitBulletLines(const string& text) {
    vector<string> lines;
    istringstream stream(text);
    string line;
    while (getline(stream, line)) {
        line.erase(line.begin(),
                   find_if(line.begin(), line.end(), [](unsigned char ch) {
                       return !isspace(ch) && ch != '-' && ch != '*' && ch != '1' && ch != '.' && ch != ')';
                   }));
        if (!line.empty()) {
            lines.push_back(line);
        }
    }
    return lines;
}

// Formats dashboard data into prompt-ready text.
string AIService::buildDashboardContext(const string& userId,
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

    ostringstream context;
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

// Formats savings data into prompt-ready text.
string AIService::buildSavingsContext(const string& userId,
                                           const models::YearMonth& period,
                                           const SavingsService& savingsService) {
    const auto savings = savingsService.summarize(userId, period);
    ostringstream context;
    context << "Period: " << period.year << "-" << period.month << "\n";
    context << "Current balance: " << savings.currentBalance << "\n";
    context << "Monthly saved: " << savings.monthlySaved << "\n";
    context << "Monthly target: " << savings.monthlyTarget << "\n";
    context << "Progress ratio: " << savings.progressToMonthlyTarget << "\n";
    context << "Long-term target: " << savings.longTermTarget << "\n";
    return context.str();
}

// Builds a simple merchant/category fallback from receipt text.
string AIService::fallbackReceiptCategoryName(const string& rawText) {
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

// Builds a compact summary of an already-generated FinancialReport for the model prompt.
string AIService::buildFinancialReportContext(const models::FinancialReport& report) {
    ostringstream context;
    context << "Period: " << report.request.from.toString() << " .. " << report.request.to.toString() << "\n";
    context << "Income: " << report.totalIncome << " EGP\n";
    context << "Expenses: " << report.totalExpenses << " EGP\n";
    context << "Net: " << report.net << " EGP\n";
    context << "Transactions in range: " << report.transactions.size() << "\n";
    context << "Expense categories:\n";
    for (const auto& line : report.categoryExpenses) {
        context << "- " << line.categoryName << ": " << line.amount << " EGP\n";
    }
    context << "Budget status lines: " << report.impactedBudgets.size() << "\n";
    for (const auto& st : report.impactedBudgets) {
        context << "- spent=" << st.spent << ", limit=" << st.budget.limit
                << ", remaining=" << st.remaining << ", overspent=" << (st.overspent ? "yes" : "no") << "\n";
    }
    context << "Sample transactions (up to 20):\n";
    const size_t cap = min<size_t>(report.transactions.size(), 20);
    for (size_t i = 0; i < cap; ++i) {
        const auto& t = report.transactions[i];
        context << "- " << t.date.toString() << " | " << (t.type == models::TransactionType::Income ? "in" : "out")
                << " | " << t.title << " | " << t.amount << " EGP\n";
    }
    return context.str();
}

// Rule-based bullets when AI is unavailable or times out.
string AIService::heuristicReportRecommendations(const models::FinancialReport& report) {
    ostringstream text;
    if (report.net < 0.0) {
        text << "- Reduce flexible spending until the net balance returns above zero.\n";
    } else if (report.net > 0.0 && report.totalIncome > 0.0) {
        text << "- Keep directing part of your surplus toward savings or goal progress.\n";
    } else if (report.totalIncome <= 0.0 && report.totalExpenses > 0.0) {
        text << "- Record expected income for this period so planning reflects reality.\n";
    }
    if (!report.categoryExpenses.empty()) {
        text << "- Review " << report.categoryExpenses.front().categoryName
             << " first; it had the largest expense share this period.\n";
    }
    bool anyOverspent = false;
    for (const auto& st : report.impactedBudgets) {
        if (st.overspent) {
            anyOverspent = true;
            break;
        }
    }
    if (anyOverspent) {
        text << "- Revisit overspent budgets and adjust limits only when the new limit reflects a real plan.\n";
    } else if (!report.impactedBudgets.empty()) {
        text << "- Budgets are on track; maintain the habits that kept spending within limits.\n";
    }
    if (report.transactions.empty()) {
        text << "- Add transactions for this period so future reports capture real activity.\n";
    }
    const string s = text.str();
    return s.empty() ? string("- Review your categories and budgets for the next period.\n") : s;
}

string AIService::normalizeReportRecommendationLines(const string& raw) {
    ostringstream out;
    istringstream stream(raw);
    string line;
    int count = 0;
    constexpr int kMaxLines = 8;
    while (getline(stream, line) && count < kMaxLines) {
        while (!line.empty() && (line.front() == ' ' || line.front() == '\t')) {
            line.erase(line.begin());
        }
        if (line.empty()) {
            continue;
        }
        if (line.front() != '-') {
            line.insert(line.begin(), '-');
            line.insert(line.begin() + 1, ' ');
        } else if (line.size() < 2 || line[1] != ' ') {
            line.insert(line.begin() + 1, ' ');
        }
        out << line << '\n';
        ++count;
    }
    return out.str();
}

string AIService::generateFinancialReportRecommendations(const models::FinancialReport& report) const {
    ostringstream section;
    section << "\nRecommended Next Steps\n";
    section << "----------------------\n";

    if (!openRouterKeyLooksConfigured(config_.apiKey)) {
        section << "(AI not configured; using built-in advice.)\n";
        section << heuristicReportRecommendations(report);
        return section.str();
    }

    network::ai::ChatCompletionClient client;
    const string context = buildFinancialReportContext(report);
    const auto response = client.complete(
        config_,
        models::AIChatRequest {
            .model = "liquid/lfm-2.5-1.2b-instruct:free",
            .messages = {
                {"system",
                 "You are a concise personal finance coach. Reply with 4 to 6 lines only. "
                 "Each line must start with exactly '- '. Mention amounts in EGP when useful. "
                 "Do not add headings or text before the first bullet."},
                {"user",
                 "Here is a FinSight user report summary for one period. Give practical next steps.\n\n" + context},
            },
            .temperature = 0.25,
        },
        10);

    const bool placeholder = response.content.find("AI placeholder") != string::npos;
    if (response.success && !response.content.empty() && !placeholder) {
        const string normalized = normalizeReportRecommendationLines(response.content);
        if (!normalized.empty()) {
            section << normalized;
            if (!response.model.empty()) {
                section << "\n(Model: " << response.model << ")\n";
            }
            return section.str();
        }
        section << "(AI reply was empty or unusable; using built-in advice.)\n";
    } else {
        section << "(AI did not return usable output within 10 seconds; using built-in advice.)\n";
    }
    section << heuristicReportRecommendations(report);
    return section.str();
}

}  // namespace finsight::core::services
