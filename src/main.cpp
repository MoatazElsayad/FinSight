#include "core/managers/FinanceTrackerBackend.h"
#include "core/utils/EnvLoader.h"
#include "data/storage/BackendStore.h"

#include <iostream>
#include <string>

using namespace std;

int main() {
    using finsight::core::managers::FinanceTrackerBackend;
    using finsight::core::models::Date;
    using finsight::core::models::EmailProviderConfig;
    using finsight::core::models::Goal;
    using finsight::core::models::Investment;
    using finsight::core::models::InvestmentType;
    using finsight::core::models::AIProviderConfig;
    using finsight::core::models::PantryItem;
    using finsight::core::models::ReceiptConfirmation;
    using finsight::core::models::ReportRequest;
    using finsight::core::models::SavingsEntry;
    using finsight::core::models::SavingsEntryType;
    using finsight::core::models::ShoppingItem;
    using finsight::core::models::Transaction;
    using finsight::core::models::TransactionType;
    using finsight::core::models::YearMonth;
    using finsight::core::utils::EnvLoader;
    using finsight::data::storage::BackendStore;

    // Loads environment values from the nearest .env file before building the AI config.
    EnvLoader::loadFromNearestFile();
    const string apiUrl = EnvLoader::get("FINSIGHT_OPENROUTER_API_URL", "https://openrouter.ai/api/v1/chat/completions");
    const string apiKey = EnvLoader::get("FINSIGHT_OPENROUTER_API_KEY", "PASTE_REAL_API_KEY_HERE");
    const string primaryModel = EnvLoader::get("FINSIGHT_OPENROUTER_MODEL", "liquid/lfm-2.5-1.2b-instruct:free");
    const bool emailEnabled = EnvLoader::getBool("FINSIGHT_EMAIL_ENABLED", false);
    const string emailApiUrl = EnvLoader::get("FINSIGHT_RESEND_API_URL", "https://api.resend.com/emails");
    const string emailApiKey = EnvLoader::get("FINSIGHT_RESEND_API_KEY", "PASTE_REAL_RESEND_API_KEY_HERE");
    const string emailFromEmail = EnvLoader::get("FINSIGHT_RESEND_FROM_EMAIL");
    const string emailFromName = EnvLoader::get("FINSIGHT_RESEND_FROM_NAME", "FinSight");

    // Builds a demo backend instance and configures the AI provider.
    FinanceTrackerBackend backend;
    backend.ai().configure(AIProviderConfig {
        .apiUrl = apiUrl,
        .apiKey = apiKey,
        .model = primaryModel,
        .fallbackModels = {
            "liquid/lfm-2.5-1.2b-thinking:free",
            "meta-llama/llama-3.3-70b-instruct:free",
            "qwen/qwen3-coder:free",
            "deepseek/deepseek-r1-0528:free",
            "mistralai/mistral-small-3.1-24b-instruct:free"
        },
        .appName = "FinSight",
        .appUrl = "https://example.com/finsight",
    });
    backend.email().configure(EmailProviderConfig {
        .enabled = emailEnabled,
        .apiUrl = emailApiUrl,
        .apiKey = emailApiKey,
        .fromEmail = emailFromEmail,
        .fromName = emailFromName,
    });
    // Creates one demo user and session for the sample run.
    const auto user = backend.auth().registerUser(
        "Demo User",
        "demo@finsight.local",
        "+20-100-000-0000",
        "Prefer not to say",
        "password123",
        Date {2026, 4, 3});
    const auto session = backend.sessions().startSession(user.id, Date {2026, 4, 3});

    // Loads built-in categories used by the rest of the demo.
    const auto categories = backend.transactions().getCategoriesForUser(user.id);
    const auto salaryCategory = categories.at(0);
    const auto foodCategory = categories.at(2);

    // Adds sample income and expense transactions.
    backend.transactions().addTransaction(Transaction {
        .userId = user.id,
        .title = "Monthly Salary",
        .description = "April salary",
        .categoryId = salaryCategory.id,
        .type = TransactionType::Income,
        .amount = 18000.0,
        .date = Date {2026, 4, 1},
        .merchant = "Employer",
    });

    const auto firstExpense = backend.transactions().addTransaction(Transaction {
        .userId = user.id,
        .title = "Groceries",
        .description = "Weekly grocery trip",
        .categoryId = foodCategory.id,
        .type = TransactionType::Expense,
        .amount = 950.0,
        .date = Date {2026, 4, 2},
        .merchant = "Local Market",
    });

    // Adds sample budgets, savings, investments, and goals.
    backend.budgets().createBudget(user.id, foodCategory.id, YearMonth {2026, 4}, 1500.0);
    backend.savings().setGoal(user.id, 5000.0, 100000.0, Date {2026, 12, 31});
    backend.savings().addEntry(SavingsEntry {
        .userId = user.id,
        .type = SavingsEntryType::Deposit,
        .amount = 2500.0,
        .date = Date {2026, 4, 3},
        .note = "Savings deposit",
    });

    backend.savings().addInvestment(Investment {
        .userId = user.id,
        .assetName = "Gold",
        .symbol = "XAU",
        .type = InvestmentType::Gold,
        .quantity = 2.0,
        .buyRate = 3200.0,
        .currentRate = 3340.0,
        .purchaseDate = Date {2026, 4, 3},
    });

    backend.goals().createGoal(Goal {
        .userId = user.id,
        .title = "Emergency Fund",
        .description = "Build a six month emergency fund",
        .targetAmount = 50000.0,
        .currentAmount = 12000.0,
        .targetDate = Date {2026, 12, 31},
    });

    // Adds sample pantry and shopping list data.
    backend.shopping().upsertPantryItem(PantryItem {
        .userId = user.id,
        .name = "Rice",
        .unit = "kg",
        .quantity = 1.0,
        .lowStockThreshold = 2.0,
        .updatedAt = Date {2026, 4, 3},
    });
    backend.shopping().addShoppingItem(ShoppingItem {
        .userId = user.id,
        .name = "Milk",
        .category = "Groceries",
        .plannedQuantity = 2.0,
        .unit = "bottles",
        .purchased = false,
        .createdAt = Date {2026, 4, 3},
    });

    // Runs the receipt import flow using sample OCR text.
    const auto receipt = backend.receipts().uploadReceipt(
        user.id,
        "market_receipt.txt",
        "Local Market\n2026-04-02\nTOTAL 950.00\nFood and groceries",
        Date {2026, 4, 3});
    const auto parsedReceipt = backend.receipts().parseReceipt(user.id, receipt.id, backend.transactions());
    const auto importedReceiptTransaction = backend.receipts().confirmReceiptAsTransaction(
        user.id,
        ReceiptConfirmation {
            .receiptId = receipt.id,
            .title = "Receipt import",
            .description = "Imported from parsed receipt",
            .merchant = parsedReceipt.merchant,
            .categoryId = parsedReceipt.suggestedCategoryId.empty() ? foodCategory.id : parsedReceipt.suggestedCategoryId,
            .type = TransactionType::Expense,
            .amount = parsedReceipt.amount.value_or(950.0),
            .date = parsedReceipt.transactionDate.value_or(Date {2026, 4, 2}),
        },
        backend.transactions());
    const auto budgetAlertEmails = backend.budgetAlerts().notifyBudgetExceededByTransaction(
        importedReceiptTransaction,
        backend.auth(),
        backend.transactions(),
        backend.budgets(),
        backend.email());

    // Builds analytics, reports, and AI outputs from the sample data.
    const auto dashboard = backend.analytics().buildDashboard(
        user.id,
        YearMonth {2026, 4},
        backend.transactions(),
        backend.budgets(),
        backend.savings(),
        backend.goals());
    const auto shoppingSnapshot = backend.shopping().snapshot(user.id);
    const auto report = backend.reports().generateReport(
        ReportRequest {
            .userId = user.id,
            .from = Date {2026, 4, 1},
            .to = Date {2026, 4, 30},
        },
        backend.transactions(),
        backend.budgets());
    const auto aiDashboard = backend.ai().generateDashboardInsight(
        user.id,
        YearMonth {2026, 4},
        backend.analytics(),
        backend.transactions(),
        backend.budgets(),
        backend.savings(),
        backend.goals());
    const auto aiSavings = backend.ai().analyzeSavings(
        user.id,
        YearMonth {2026, 4},
        backend.savings());
    const auto aiChat = backend.ai().answerFinanceQuestion(
        user.id,
        "What should I improve this month?",
        YearMonth {2026, 4},
        backend.analytics(),
        backend.transactions(),
        backend.budgets(),
        backend.savings(),
        backend.goals());
    const auto aiReceipt = backend.ai().suggestReceiptTransaction(
        "Local Market\n2026-04-02\nTOTAL 950.00\nFood and groceries",
        "Local Market");

    // Saves the backend state and reloads it into a fresh instance.
    BackendStore store;
    const auto persistenceDirectory = filesystem::path {"runtime_data"};
    store.save(backend, persistenceDirectory);

    FinanceTrackerBackend restoredBackend;
    store.load(restoredBackend, persistenceDirectory);
    const auto restoredDashboard = restoredBackend.analytics().buildDashboard(
        user.id,
        YearMonth {2026, 4},
        restoredBackend.transactions(),
        restoredBackend.budgets(),
        restoredBackend.savings(),
        restoredBackend.goals());

    // Prints a compact summary so the demo results are easy to inspect.
    cout << "FinSight backend demo\n";
    cout << "Session token: " << session.token << '\n';
    cout << "Income: " << dashboard.overview.income << '\n';
    cout << "Expenses: " << dashboard.overview.expenses << '\n';
    cout << "Net savings: " << dashboard.overview.netSavings << '\n';
    cout << "Savings balance: " << dashboard.savings.currentBalance << '\n';
    cout << "Active goals: " << dashboard.activeGoals.size() << '\n';
    cout << "Low stock items: " << shoppingSnapshot.lowStockItems.size() << '\n';
    cout << "Report net: " << report.net << '\n';
    cout << "AI dashboard fallback: " << (aiDashboard.usedFallback ? "yes" : "no") << '\n';
    cout << "AI dashboard summary: " << aiDashboard.summary << '\n';
    cout << "AI savings fallback: " << (aiSavings.usedFallback ? "yes" : "no") << '\n';
    cout << "AI savings summary: " << aiSavings.summary << '\n';
    cout << "AI chat answer: " << aiChat.answer << '\n';
    cout << "AI receipt notes: " << aiReceipt.parseResult.confidenceNotes << '\n';
    cout << "AI models tried: " << backend.ai().config().model;
    for (const auto& model : backend.ai().config().fallbackModels) {
        cout << ", " << model;
    }
    cout << '\n';
    cout << "AI URL: " << backend.ai().config().apiUrl << '\n';
    cout << "Email enabled: " << (backend.email().config().enabled ? "yes" : "no") << '\n';
    cout << "Budget alert emails attempted: " << budgetAlertEmails.size() << '\n';
    for (const auto& email : budgetAlertEmails) {
        cout << "Budget alert to " << email.recipient << ": " << (email.success ? "sent" : "not sent") << '\n';
        if (!email.error.empty()) {
            cout << "Email detail: " << email.error << '\n';
        }
    }
    cout << "SQLite file: " << store.databasePath(persistenceDirectory).string() << '\n';
    cout << "JSON sidecar: " << store.sidecarPath(persistenceDirectory).string() << '\n';
    cout << "Restored expenses: " << restoredDashboard.overview.expenses << '\n';

    return 0;
}
