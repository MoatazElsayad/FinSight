#include "core/managers/FinanceTrackerBackend.h"
#include "data/storage/BackendStore.h"

#include <cstdlib>
#include <iostream>
#include <string>

int main() {
    using finsight::core::managers::FinanceTrackerBackend;
    using finsight::core::models::Date;
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
    using finsight::data::storage::BackendStore;

    const char* apiKeyValue = std::getenv("FINSIGHT_OPENROUTER_API_KEY");
    const std::string apiKey = apiKeyValue == nullptr ? "PASTE_REAL_API_KEY_HERE" : apiKeyValue;

    FinanceTrackerBackend backend;
    backend.ai().configure(AIProviderConfig {
        .apiUrl = "https://openrouter.ai/api/v1/chat/completions",
        .apiKey = apiKey,
        .model = "liquid/lfm-2.5-1.2b-instruct:free",
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
    const auto user = backend.auth().registerUser(
        "Demo User",
        "demo@finsight.local",
        "+20-100-000-0000",
        "Prefer not to say",
        "password123",
        Date {2026, 4, 3});
    const auto session = backend.sessions().startSession(user.id, Date {2026, 4, 3});

    const auto categories = backend.transactions().getCategoriesForUser(user.id);
    const auto salaryCategory = categories.at(0);
    const auto foodCategory = categories.at(2);

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

    backend.transactions().addTransaction(Transaction {
        .userId = user.id,
        .title = "Groceries",
        .description = "Weekly grocery trip",
        .categoryId = foodCategory.id,
        .type = TransactionType::Expense,
        .amount = 950.0,
        .date = Date {2026, 4, 2},
        .merchant = "Local Market",
    });

    backend.budgets().createBudget(user.id, foodCategory.id, YearMonth {2026, 4}, 3000.0);
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

    const auto receipt = backend.receipts().uploadReceipt(
        user.id,
        "market_receipt.txt",
        "Local Market\n2026-04-02\nTOTAL 950.00\nFood and groceries",
        Date {2026, 4, 3});
    const auto parsedReceipt = backend.receipts().parseReceipt(user.id, receipt.id, backend.transactions());
    backend.receipts().confirmReceiptAsTransaction(
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

    BackendStore store;
    store.save(backend, "runtime_data");

    FinanceTrackerBackend restoredBackend;
    store.load(restoredBackend, "runtime_data");
    const auto restoredDashboard = restoredBackend.analytics().buildDashboard(
        user.id,
        YearMonth {2026, 4},
        restoredBackend.transactions(),
        restoredBackend.budgets(),
        restoredBackend.savings(),
        restoredBackend.goals());

    std::cout << "FinSight backend demo\n";
    std::cout << "Session token: " << session.token << '\n';
    std::cout << "Income: " << dashboard.overview.income << '\n';
    std::cout << "Expenses: " << dashboard.overview.expenses << '\n';
    std::cout << "Net savings: " << dashboard.overview.netSavings << '\n';
    std::cout << "Savings balance: " << dashboard.savings.currentBalance << '\n';
    std::cout << "Active goals: " << dashboard.activeGoals.size() << '\n';
    std::cout << "Low stock items: " << shoppingSnapshot.lowStockItems.size() << '\n';
    std::cout << "Report net: " << report.net << '\n';
    std::cout << "AI dashboard fallback: " << (aiDashboard.usedFallback ? "yes" : "no") << '\n';
    std::cout << "AI dashboard summary: " << aiDashboard.summary << '\n';
    std::cout << "AI savings fallback: " << (aiSavings.usedFallback ? "yes" : "no") << '\n';
    std::cout << "AI savings summary: " << aiSavings.summary << '\n';
    std::cout << "AI chat answer: " << aiChat.answer << '\n';
    std::cout << "AI receipt notes: " << aiReceipt.parseResult.confidenceNotes << '\n';
    std::cout << "AI models tried: " << backend.ai().config().model;
    for (const auto& model : backend.ai().config().fallbackModels) {
        std::cout << ", " << model;
    }
    std::cout << '\n';
    std::cout << "Restored expenses: " << restoredDashboard.overview.expenses << '\n';

    return 0;
}
