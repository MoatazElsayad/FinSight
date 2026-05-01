#include "BudgetAlertService.h"

#include "AIService.h"
#include "AuthService.h"
#include "BudgetService.h"
#include "EmailService.h"
#include "TransactionService.h"
#include "../../network/ai/ChatCompletionClient.h"

#include <chrono>
#include <future>
#include <iomanip>
#include <memory>
#include <sstream>
#include <thread>

using namespace std;

namespace finsight::core::services {

namespace {

string formatMoneyValue(double value) {
    ostringstream stream;
    stream << fixed << setprecision(2) << value;
    return stream.str();
}

string fallbackBudgetAdvice(const string& categoryName, double overAmount) {
    ostringstream advice;
    advice << "For the next week, pause non-essential " << categoryName
           << " spending and set a small daily cap before making new purchases. "
           << "Move " << formatMoneyValue(overAmount)
           << " from a flexible category or reduce upcoming expenses until this budget is back under control.";
    return advice.str();
}

struct BudgetAdvice {
    string text;
    bool aiGenerated {false};
    string modelName;
};

string modelDisplayName(const string& model) {
    if (model.find("lfm") != string::npos) {
        return "LFM";
    }
    if (model.find("minimax") != string::npos) {
        return "Minimax";
    }
    return model;
}

bool looksLikePlaceholder(const string& text) {
    return text.find("placeholder response") != string::npos ||
           text.find("OPENROUTER_API_KEY") != string::npos;
}

string requestAiBudgetAdvice(const models::AIProviderConfig& config,
                             const string& model,
                             const string& categoryName,
                             double budgetLimit,
                             double currentSpending,
                             double overAmount,
                             const models::Transaction& transaction) {
    network::ai::ChatCompletionClient client;

    ostringstream prompt;
    prompt << "A user exceeded their " << categoryName << " monthly budget. "
           << "Budget limit: " << formatMoneyValue(budgetLimit) << ". "
           << "Current spending: " << formatMoneyValue(currentSpending) << ". "
           << "Amount over budget: " << formatMoneyValue(overAmount) << ". "
           << "Latest transaction: " << transaction.title
           << " for " << formatMoneyValue(transaction.amount) << ". "
           << "Write exactly two short practical sentences telling them how to control this budget. "
           << "Do not use bullets.";

    auto response = client.complete(config, models::AIChatRequest {
        .model = model,
        .messages = {
            {"system", "You are a concise personal finance coach."},
            {"user", prompt.str()},
        },
        .temperature = 0.2,
    });

    if (!response.success || response.content.empty() || looksLikePlaceholder(response.content)) {
        return {};
    }
    return response.content;
}

string requestAiBudgetAdviceWithinTimeout(const models::AIProviderConfig& config,
                                          const string& model,
                                          const string& categoryName,
                                          double budgetLimit,
                                          double currentSpending,
                                          double overAmount,
                                          const models::Transaction& transaction,
                                          chrono::seconds timeout) {
    auto promise = make_shared<std::promise<string>>();
    auto future = promise->get_future();

    std::thread([promise, config, model, categoryName, budgetLimit, currentSpending, overAmount, transaction]() {
        try {
            promise->set_value(requestAiBudgetAdvice(config,
                                                     model,
                                                     categoryName,
                                                     budgetLimit,
                                                     currentSpending,
                                                     overAmount,
                                                     transaction));
        } catch (...) {
            try {
                promise->set_value({});
            } catch (...) {
            }
        }
    }).detach();

    if (future.wait_for(timeout) != std::future_status::ready) {
        return {};
    }
    return future.get();
}

BudgetAdvice budgetAdviceWithinTimeout(const AIService& aiService,
                                       const string& categoryName,
                                       double budgetLimit,
                                       double currentSpending,
                                       double overAmount,
                                       const models::Transaction& transaction) {
    const auto config = aiService.config();
    const vector<string> adviceModels {
        "liquid/lfm-2.5-1.2b-instruct:free",
        "minimax/minimax-m2.5:free",
    };

    for (const auto& model : adviceModels) {
        auto advice = requestAiBudgetAdviceWithinTimeout(config,
                                                         model,
                                                         categoryName,
                                                         budgetLimit,
                                                         currentSpending,
                                                         overAmount,
                                                         transaction,
                                                         std::chrono::seconds(15));
        if (!advice.empty()) {
            return BudgetAdvice {
                .text = advice,
                .aiGenerated = true,
                .modelName = modelDisplayName(model),
            };
        }
    }

    return BudgetAdvice {
        .text = fallbackBudgetAdvice(categoryName, overAmount),
        .aiGenerated = false,
    };
}

string escapeHtml(const string& value) {
    string escaped;
    for (char ch : value) {
        switch (ch) {
        case '&':
            escaped += "&amp;";
            break;
        case '<':
            escaped += "&lt;";
            break;
        case '>':
            escaped += "&gt;";
            break;
        case '"':
            escaped += "&quot;";
            break;
        case '\'':
            escaped += "&#39;";
            break;
        case '\n':
            escaped += "<br>";
            break;
        default:
            escaped.push_back(ch);
            break;
        }
    }
    return escaped;
}

string buildBudgetAlertHtml(const string& fullName,
                            const string& categoryName,
                            const models::BudgetStatus& status,
                            const models::Transaction& transaction,
                            double overAmount,
                            const BudgetAdvice& advice) {
    ostringstream html;
    const string period = to_string(status.budget.period.year) + "-" +
                          (status.budget.period.month < 10 ? "0" : "") +
                          to_string(status.budget.period.month);
    const string adviceTitle = advice.aiGenerated
                                   ? "FinSight AI advice from " + advice.modelName
                                   : "FinSight backup advice";

    html << "<!doctype html><html><body style=\"margin:0;padding:0;background:#f4f7fb;"
         << "font-family:Arial,Helvetica,sans-serif;color:#172033;\">";
    html << "<div style=\"max-width:640px;margin:0 auto;padding:28px 18px;\">";
    html << "<div style=\"background:#101827;color:#ffffff;border-radius:14px 14px 0 0;padding:24px 26px;\">";
    html << "<div style=\"font-size:13px;letter-spacing:.08em;text-transform:uppercase;color:#8ee6b8;"
         << "font-weight:700;\">FinSight Budget Alert</div>";
    html << "<h1 style=\"margin:10px 0 0;font-size:24px;line-height:1.25;\">&#9888; "
         << escapeHtml(categoryName) << " budget exceeded</h1>";
    html << "</div>";
    html << "<div style=\"background:#ffffff;border:1px solid #dfe7f3;border-top:0;"
         << "border-radius:0 0 14px 14px;padding:24px 26px;\">";
    html << "<p style=\"margin:0 0 16px;font-size:16px;\">Hello <strong>"
         << escapeHtml(fullName) << "</strong>,</p>";
    html << "<p style=\"margin:0 0 22px;font-size:15px;line-height:1.55;\">Your <strong>"
         << escapeHtml(categoryName) << "</strong> budget for <strong>" << period
         << "</strong> has been exceeded. Here is the quick summary.</p>";
    html << "<table role=\"presentation\" style=\"width:100%;border-collapse:collapse;margin:0 0 22px;\">";
    html << "<tr><td style=\"padding:10px;border-bottom:1px solid #eef2f7;\">Budget limit</td>"
         << "<td style=\"padding:10px;border-bottom:1px solid #eef2f7;text-align:right;\"><strong>"
         << formatMoneyValue(status.budget.limit) << "</strong></td></tr>";
    html << "<tr><td style=\"padding:10px;border-bottom:1px solid #eef2f7;\">Current spending</td>"
         << "<td style=\"padding:10px;border-bottom:1px solid #eef2f7;text-align:right;\"><strong>"
         << formatMoneyValue(status.spent) << "</strong></td></tr>";
    html << "<tr><td style=\"padding:10px;border-bottom:1px solid #eef2f7;\">Amount over budget</td>"
         << "<td style=\"padding:10px;border-bottom:1px solid #eef2f7;text-align:right;color:#c62828;\"><strong>"
         << formatMoneyValue(overAmount) << "</strong></td></tr>";
    html << "<tr><td style=\"padding:10px;border-bottom:1px solid #eef2f7;\">Latest transaction</td>"
         << "<td style=\"padding:10px;border-bottom:1px solid #eef2f7;text-align:right;\"><strong>"
         << escapeHtml(transaction.title) << " (" << formatMoneyValue(transaction.amount)
         << ")</strong></td></tr>";
    html << "<tr><td style=\"padding:10px;\">Transaction date</td>"
         << "<td style=\"padding:10px;text-align:right;\"><strong>"
         << escapeHtml(transaction.date.toString()) << "</strong></td></tr>";
    html << "</table>";
    html << "<div style=\"background:#ecfdf3;border:1px solid #b8efcf;border-radius:12px;"
         << "padding:16px 18px;margin:0 0 20px;\">";
    html << "<div style=\"font-weight:700;color:#0c6b3b;margin-bottom:8px;\">&#128161; "
         << adviceTitle << "</div>";
    html << "<div style=\"font-size:15px;line-height:1.6;\">" << escapeHtml(advice.text) << "</div>";
    html << "</div>";
    html << "<p style=\"margin:0;font-size:14px;color:#536176;\">Open FinSight to review your spending, "
         << "adjust the budget, or move money from a flexible category.</p>";
    html << "</div></div></body></html>";
    return html.str();
}

}  // namespace

// Sends one alert email if the supplied expense transaction newly crosses a budget limit.
vector<models::EmailSendResult> BudgetAlertService::notifyBudgetExceededByTransaction(
    const models::Transaction& transaction,
    const AIService& aiService,
    const AuthService& authService,
    const TransactionService& transactionService,
    const BudgetService& budgetService,
    const EmailService& emailService) const {
    vector<models::EmailSendResult> results;

    if (!emailService.config().enabled || transaction.type != models::TransactionType::Expense) {
        return results;
    }

    const models::YearMonth period {transaction.date.year, transaction.date.month};
    const auto statuses = budgetService.summarizeBudgets(transaction.userId, period, transactionService);

    for (const auto& status : statuses) {
        if (status.budget.categoryId != transaction.categoryId || !status.overspent) {
            continue;
        }

        const double previousSpent = status.spent - transaction.amount;
        if (previousSpent > status.budget.limit) {
            continue;
        }

        const auto& user = authService.getUser(transaction.userId);
        const auto& category = transactionService.requireCategory(transaction.categoryId);
        const double overAmount = status.spent - status.budget.limit;
        const BudgetAdvice advice = budgetAdviceWithinTimeout(aiService,
                                                              category.name,
                                                              status.budget.limit,
                                                              status.spent,
                                                              overAmount,
                                                              transaction);

        ostringstream subject;
        subject << "FinSight budget alert: " << category.name << " budget exceeded";

        ostringstream body;
        body << "Hello " << user.fullName << ",\n\n";
        body << "Your " << category.name << " budget for "
             << status.budget.period.year << '-'
             << setw(2) << setfill('0') << status.budget.period.month
             << " has been exceeded.\n\n";
        body << "Budget limit: " << formatMoney(status.budget.limit) << "\n";
        body << "Current spending: " << formatMoney(status.spent) << "\n";
        body << "Amount over budget: " << formatMoney(overAmount) << "\n";
        body << "Latest transaction: " << transaction.title << " (" << formatMoney(transaction.amount) << ")\n";
        body << "Transaction date: " << transaction.date.toString() << "\n\n";
        body << (advice.aiGenerated
                     ? "FinSight AI advice from " + advice.modelName + ":\n"
                     : "FinSight backup advice:\n");
        body << advice.text << "\n\n";
        body << "Open FinSight to review your spending and adjust your budget if needed.\n";

        results.push_back(emailService.sendEmail(models::EmailMessage {
            .to = user.email,
            .subject = subject.str(),
            .body = body.str(),
            .htmlBody = buildBudgetAlertHtml(user.fullName,
                                             category.name,
                                             status,
                                             transaction,
                                             overAmount,
                                             advice),
        }));
    }

    return results;
}

// Formats money values with two decimal places for email messages.
string BudgetAlertService::formatMoney(double value) {
    ostringstream stream;
    stream << fixed << setprecision(2) << value;
    return stream.str();
}

}  // namespace finsight::core::services
