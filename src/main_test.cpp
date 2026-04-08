#include "core/managers/FinanceTrackerBackend.h"
#include "core/utils/EnvLoader.h"
#include "data/storage/BackendStore.h"

#include <filesystem>
#include <iomanip>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>

using namespace std;

namespace {

using finsight::core::managers::FinanceTrackerBackend;
using finsight::core::models::AIProviderConfig;
using finsight::core::models::Date;
using finsight::core::models::EmailProviderConfig;
using finsight::core::models::Goal;
using finsight::core::models::ReportRequest;
using finsight::core::models::SavingsEntry;
using finsight::core::models::SavingsEntryType;
using finsight::core::models::Transaction;
using finsight::core::models::TransactionType;
using finsight::core::models::YearMonth;
using finsight::core::utils::EnvLoader;
using finsight::data::storage::BackendStore;

const filesystem::path kPersistenceDirectory {"runtime_data_terminal_demo"};

// Draws a simple full-width separator.
void printSeparator() {
    cout << string(78, '=') << '\n';
}

// Prints a page header so the CLI feels like screens in the future app.
void printPage(const string& title, const string& subtitle = "") {
    printSeparator();
    cout << title << '\n';
    if (!subtitle.empty()) {
        cout << subtitle << '\n';
    }
    printSeparator();
}

// Prints one labeled value line.
void printField(const string& label, const string& value) {
    cout << left << setw(22) << label << ": " << value << '\n';
}

// Reads one full input line from the user.
string promptLine(const string& label) {
    cout << label;
    string value;
    getline(cin, value);
    return value;
}

// Reads a menu selection line.
string promptChoice(const string& label) {
    cout << '\n' << label;
    string choice;
    getline(cin, choice);
    return choice;
}

// Reads a double value from the user.
double promptDouble(const string& label) {
    while (true) {
        const string value = promptLine(label);
        try {
            return stod(value);
        } catch (...) {
            cout << "Please enter a valid number.\n";
        }
    }
}

// Reads a date from the user in YYYY-MM-DD format.
Date promptDate(const string& label) {
    while (true) {
        const string value = promptLine(label);
        try {
            return Date::fromString(value);
        } catch (...) {
            cout << "Please enter a valid date in YYYY-MM-DD format.\n";
        }
    }
}

// Returns all categories available to the current user.
vector<finsight::core::models::Category> categoriesForUser(const FinanceTrackerBackend& backend, const string& userId) {
    return backend.transactions().getCategoriesForUser(userId);
}

// Lets the user choose one category by number.
string chooseCategory(FinanceTrackerBackend& backend,
                      const string& userId,
                      finsight::core::models::CategoryKind kind) {
    vector<finsight::core::models::Category> matching;
    for (const auto& category : categoriesForUser(backend, userId)) {
        if (category.kind == kind) {
            matching.push_back(category);
        }
    }

    while (true) {
        cout << "\nChoose category\n";
        for (size_t index = 0; index < matching.size(); ++index) {
            cout << "  [" << index + 1 << "] " << matching[index].name << '\n';
        }
        cout << "  [C] Create custom category\n";

        const string choice = promptChoice("Choice: ");
        if (choice == "C" || choice == "c") {
            const string name = promptLine("New category name: ");
            const string icon = promptLine("Icon/emoji or text icon: ");
            return backend.transactions().createCategory(userId, name, kind, icon).id;
        }

        try {
            const size_t index = static_cast<size_t>(stoul(choice));
            if (index >= 1 && index <= matching.size()) {
                return matching[index - 1].id;
            }
        } catch (...) {
        }
        cout << "Invalid choice.\n";
    }
}

// Configures the AI provider from environment settings.
void configureAi(FinanceTrackerBackend& backend) {
    EnvLoader::loadFromNearestFile();
    const string apiUrl = EnvLoader::get("FINSIGHT_OPENROUTER_API_URL", "https://openrouter.ai/api/v1/chat/completions");
    const string apiKey = EnvLoader::get("FINSIGHT_OPENROUTER_API_KEY", "PASTE_REAL_API_KEY_HERE");
    const string primaryModel = EnvLoader::get("FINSIGHT_OPENROUTER_MODEL", "liquid/lfm-2.5-1.2b-instruct:free");

    backend.ai().configure(AIProviderConfig {
        .apiUrl = apiUrl,
        .apiKey = apiKey,
        .model = primaryModel,
        .fallbackModels = {
            "liquid/lfm-2.5-1.2b-thinking:free",
            "meta-llama/llama-3.3-70b-instruct:free",
            "qwen/qwen3-coder:free",
            "deepseek/deepseek-r1-0528:free"
        },
        .appName = "FinSight",
        .appUrl = "https://example.com/finsight",
    });
}

// Configures Resend email sending from environment settings.
void configureEmail(FinanceTrackerBackend& backend) {
    EnvLoader::loadFromNearestFile();
    backend.email().configure(EmailProviderConfig {
        .enabled = EnvLoader::getBool("FINSIGHT_EMAIL_ENABLED", false),
        .apiUrl = EnvLoader::get("FINSIGHT_RESEND_API_URL", "https://api.resend.com/emails"),
        .apiKey = EnvLoader::get("FINSIGHT_RESEND_API_KEY", "PASTE_REAL_RESEND_API_KEY_HERE"),
        .fromEmail = EnvLoader::get("FINSIGHT_RESEND_FROM_EMAIL"),
        .fromName = EnvLoader::get("FINSIGHT_RESEND_FROM_NAME", "FinSight"),
    });
}

// Saves the current backend state to disk.
void saveData(FinanceTrackerBackend& backend, BackendStore& store) {
    store.save(backend, kPersistenceDirectory);
    cout << "Data saved.\n";
    printField("SQLite File", store.databasePath(kPersistenceDirectory).string());
    printField("JSON Sidecar", store.sidecarPath(kPersistenceDirectory).string());
}

// Shows the dashboard page for the logged-in user.
void showDashboardPage(FinanceTrackerBackend& backend, const string& userId) {
    const auto dashboard = backend.analytics().buildDashboard(
        userId,
        YearMonth {2026, 4},
        backend.transactions(),
        backend.budgets(),
        backend.savings(),
        backend.goals());

    printPage("Dashboard", "Overview for the current user");
    printField("Income", to_string(dashboard.overview.income));
    printField("Expenses", to_string(dashboard.overview.expenses));
    printField("Net Savings", to_string(dashboard.overview.netSavings));
    printField("Savings Balance", to_string(dashboard.savings.currentBalance));
    printField("Goals", to_string(dashboard.activeGoals.size()));
    cout << "\nTop Expense Categories\n";
    for (const auto& item : dashboard.topExpenseCategories) {
        cout << "  - " << item.categoryName << " : " << item.amount << '\n';
    }
}

// Shows all saved transactions for the logged-in user.
void showTransactionsPage(FinanceTrackerBackend& backend, const string& userId) {
    printPage("Transactions Page", "All saved income and expenses");
    const auto transactions = backend.transactions().listTransactions(userId);
    if (transactions.empty()) {
        cout << "No transactions saved yet.\n";
        return;
    }

    for (const auto& transaction : transactions) {
        cout << "  - " << transaction.date.toString()
             << " | " << (transaction.type == TransactionType::Income ? "Income " : "Expense")
             << " | " << setw(18) << left << transaction.title
             << " | amount = " << transaction.amount
             << " | merchant = " << transaction.merchant << '\n';
    }
}

// Creates one user-entered income or expense transaction.
void addTransactionPage(FinanceTrackerBackend& backend, const string& userId) {
    printPage("Add Transaction", "Enter a new income or expense");
    const string typeChoice = promptChoice("[1] Income  [2] Expense\nChoice: ");
    const TransactionType type = typeChoice == "1" ? TransactionType::Income : TransactionType::Expense;
    const auto kind = type == TransactionType::Income
                          ? finsight::core::models::CategoryKind::Income
                          : finsight::core::models::CategoryKind::Expense;
    const string categoryId = chooseCategory(backend, userId, kind);

    const auto transaction = backend.transactions().addTransaction(Transaction {
        .userId = userId,
        .title = promptLine("Title: "),
        .description = promptLine("Description: "),
        .categoryId = categoryId,
        .type = type,
        .amount = promptDouble("Amount: "),
        .date = promptDate("Date (YYYY-MM-DD): "),
        .merchant = promptLine("Merchant/source: "),
    });

    cout << "Transaction saved.\n";
    const auto alertEmails = backend.budgetAlerts().notifyBudgetExceededByTransaction(
        transaction,
        backend.auth(),
        backend.transactions(),
        backend.budgets(),
        backend.email());
    if (alertEmails.empty()) {
        cout << "No budget alert email was needed.\n";
        return;
    }

    for (const auto& email : alertEmails) {
        cout << "Budget alert email for " << email.recipient << ": "
             << (email.success ? "sent" : "not sent") << '\n';
        if (!email.error.empty()) {
            cout << "Email detail: " << email.error << '\n';
        }
    }
}

// Shows monthly budgets.
void showBudgetsPage(FinanceTrackerBackend& backend, const string& userId) {
    printPage("Budgets Page", "Current monthly budgets");
    const auto budgetStatus = backend.budgets().summarizeBudgets(
        userId,
        YearMonth {2026, 4},
        backend.transactions());
    if (budgetStatus.empty()) {
        cout << "No budgets saved yet.\n";
        return;
    }

    for (const auto& item : budgetStatus) {
        cout << "  - category = " << item.budget.categoryId
             << " | limit = " << item.budget.limit
             << " | spent = " << item.spent
             << " | remaining = " << item.remaining << '\n';
    }
}

// Creates one monthly budget for an expense category.
void addBudgetPage(FinanceTrackerBackend& backend, const string& userId) {
    printPage("Add Budget", "Create a monthly spending budget");
    const string categoryId = chooseCategory(backend, userId, finsight::core::models::CategoryKind::Expense);
    const int year = static_cast<int>(promptDouble("Year: "));
    const int month = static_cast<int>(promptDouble("Month (1-12): "));
    const double limit = promptDouble("Budget limit: ");
    backend.budgets().createBudget(userId, categoryId, YearMonth {year, month}, limit);
    cout << "Budget saved.\n";
}

// Shows the savings page.
void showSavingsPage(FinanceTrackerBackend& backend, const string& userId) {
    printPage("Savings Page", "Savings totals and entries");
    const auto overview = backend.savings().summarize(userId, YearMonth {2026, 4});
    printField("Current Balance", to_string(overview.currentBalance));
    printField("Monthly Saved", to_string(overview.monthlySaved));
    printField("Monthly Target", to_string(overview.monthlyTarget));
    printField("Long-Term Target", to_string(overview.longTermTarget));
}

// Adds one deposit or withdrawal to savings.
void addSavingsEntryPage(FinanceTrackerBackend& backend, const string& userId) {
    printPage("Savings Entry", "Add a deposit or withdrawal");
    const string typeChoice = promptChoice("[1] Deposit  [2] Withdrawal\nChoice: ");
    backend.savings().addEntry(SavingsEntry {
        .userId = userId,
        .type = typeChoice == "2" ? SavingsEntryType::Withdrawal : SavingsEntryType::Deposit,
        .amount = promptDouble("Amount: "),
        .date = promptDate("Date (YYYY-MM-DD): "),
        .note = promptLine("Note: "),
    });
    cout << "Savings entry saved.\n";
}

// Creates or updates the user's savings goals.
void setSavingsGoalPage(FinanceTrackerBackend& backend, const string& userId) {
    printPage("Savings Goal", "Set monthly and long-term targets");
    const double monthlyTarget = promptDouble("Monthly target: ");
    const double longTermTarget = promptDouble("Long-term target: ");
    const Date targetDate = promptDate("Target date (YYYY-MM-DD): ");
    backend.savings().setGoal(userId, monthlyTarget, longTermTarget, targetDate);
    cout << "Savings goal saved.\n";
}

// Shows all saved finance goals.
void showGoalsPage(FinanceTrackerBackend& backend, const string& userId) {
    printPage("Goals Page", "Your financial goals");
    const auto goals = backend.goals().listGoals(userId);
    if (goals.empty()) {
        cout << "No goals saved yet.\n";
        return;
    }
    for (const auto& goal : goals) {
        cout << "  - " << goal.title
             << " | current = " << goal.currentAmount
             << " / target = " << goal.targetAmount
             << " | due = " << goal.targetDate.toString() << '\n';
    }
}

// Adds one new financial goal.
void addGoalPage(FinanceTrackerBackend& backend, const string& userId) {
    printPage("Add Goal", "Create a new financial goal");
    backend.goals().createGoal(Goal {
        .userId = userId,
        .title = promptLine("Goal title: "),
        .description = promptLine("Description: "),
        .targetAmount = promptDouble("Target amount: "),
        .currentAmount = promptDouble("Current progress amount: "),
        .targetDate = promptDate("Target date (YYYY-MM-DD): "),
    });
    cout << "Goal saved.\n";
}

// Shows a simple report preview.
void showReportsPage(FinanceTrackerBackend& backend, const string& userId) {
    printPage("Reports Page", "Generated report preview");
    const auto report = backend.reports().generateReport(
        ReportRequest {
            .userId = userId,
            .from = Date {2026, 4, 1},
            .to = Date {2026, 4, 30},
        },
        backend.transactions(),
        backend.budgets());
    cout << report.exportedText << '\n';
}

// Shows AI responses from the current saved user data.
void showAiPage(FinanceTrackerBackend& backend, const string& userId) {
    printPage("AI Page", "Finance chat and insights");
    const auto dashboardInsight = backend.ai().generateDashboardInsight(
        userId,
        YearMonth {2026, 4},
        backend.analytics(),
        backend.transactions(),
        backend.budgets(),
        backend.savings(),
        backend.goals());
    const auto chatAnswer = backend.ai().answerFinanceQuestion(
        userId,
        "How can I improve my finances this month?",
        YearMonth {2026, 4},
        backend.analytics(),
        backend.transactions(),
        backend.budgets(),
        backend.savings(),
        backend.goals());

    printField("Dashboard Fallback", dashboardInsight.usedFallback ? "yes" : "no");
    printField("Provider URL", backend.ai().config().apiUrl);
    printField("Primary Model", backend.ai().config().model);
    cout << "Dashboard Insight\n  " << dashboardInsight.summary << "\n\n";
    cout << "Finance Chat\n  " << chatAnswer.answer << '\n';
}

// Shows the current email configuration status.
void showEmailPage(FinanceTrackerBackend& backend) {
    printPage("Email Page", "Resend API configuration used for budget alerts");
    printField("Email Enabled", backend.email().config().enabled ? "yes" : "no");
    printField("API URL", backend.email().config().apiUrl);
    printField("From Email", backend.email().config().fromEmail.empty() ? "(not set)" : backend.email().config().fromEmail);
    printField("From Name", backend.email().config().fromName.empty() ? "(not set)" : backend.email().config().fromName);
}

// Shows the main post-login navigation shell.
void userMenu(FinanceTrackerBackend& backend, BackendStore& store, const finsight::core::models::User& user) {
    while (true) {
        printPage("FinSight App Shell", "Dashboard | Transactions | Budgets | Savings | Goals | Reports | AI | Email | Save");
        printField("Signed In User", user.fullName);
        printField("Email", user.email);
        cout << '\n';
        cout << "[1] Dashboard\n";
        cout << "[2] Transactions\n";
        cout << "[3] Add Transaction\n";
        cout << "[4] Budgets\n";
        cout << "[5] Add Budget\n";
        cout << "[6] Savings\n";
        cout << "[7] Add Savings Entry\n";
        cout << "[8] Set Savings Goal\n";
        cout << "[9] Goals\n";
        cout << "[10] Add Goal\n";
        cout << "[11] Reports\n";
        cout << "[12] AI\n";
        cout << "[13] Email\n";
        cout << "[14] Save Data\n";
        cout << "[15] Logout\n";

        const string choice = promptChoice("Choice: ");
        if (choice == "1") showDashboardPage(backend, user.id);
        else if (choice == "2") showTransactionsPage(backend, user.id);
        else if (choice == "3") addTransactionPage(backend, user.id);
        else if (choice == "4") showBudgetsPage(backend, user.id);
        else if (choice == "5") addBudgetPage(backend, user.id);
        else if (choice == "6") showSavingsPage(backend, user.id);
        else if (choice == "7") addSavingsEntryPage(backend, user.id);
        else if (choice == "8") setSavingsGoalPage(backend, user.id);
        else if (choice == "9") showGoalsPage(backend, user.id);
        else if (choice == "10") addGoalPage(backend, user.id);
        else if (choice == "11") showReportsPage(backend, user.id);
        else if (choice == "12") showAiPage(backend, user.id);
        else if (choice == "13") showEmailPage(backend);
        else if (choice == "14") saveData(backend, store);
        else if (choice == "15") {
            saveData(backend, store);
            break;
        } else {
            cout << "Invalid choice.\n";
        }

        promptChoice("\nPress Enter to continue...");
    }
}

// Registers a new user from real terminal input.
finsight::core::models::User registerUserFlow(FinanceTrackerBackend& backend) {
    printPage("Register Page", "Create a new FinSight account");
    const auto user = backend.auth().registerUser(
        promptLine("Full name: "),
        promptLine("Email: "),
        promptLine("Phone: "),
        promptLine("Gender: "),
        promptLine("Password: "),
        promptDate("Created date (YYYY-MM-DD): "));
    cout << "Registration successful.\n";
    printField("User ID", user.id);
    return user;
}

// Logs in an existing user and starts a session.
optional<finsight::core::models::User> loginUserFlow(FinanceTrackerBackend& backend) {
    printPage("Login Page", "Sign in to your saved data");
    const string email = promptLine("Email: ");
    const string password = promptLine("Password: ");
    const auto user = backend.auth().login(email, password);
    if (!user) {
        cout << "Login failed. Invalid email or password.\n";
        return nullopt;
    }
    const auto session = backend.sessions().startSession(user->id, Date {2026, 4, 12});
    cout << "Login successful.\n";
    printField("Session Token", session.token);
    return user;
}

}  // namespace

int main() {
    try {
        FinanceTrackerBackend backend;
        BackendStore store;
        configureAi(backend);
        configureEmail(backend);
        store.load(backend, kPersistenceDirectory);

        while (true) {
            printPage("FinSight | Landing Page", "Interactive backend terminal demo");
            cout << "Your entered data is loaded from and saved to disk.\n\n";
            cout << "[1] Register\n";
            cout << "[2] Login\n";
            cout << "[3] Exit\n";

            const string choice = promptChoice("Choice: ");
            if (choice == "1") {
                const auto user = registerUserFlow(backend);
                saveData(backend, store);
                userMenu(backend, store, user);
            } else if (choice == "2") {
                const auto user = loginUserFlow(backend);
                if (user) {
                    userMenu(backend, store, *user);
                }
            } else if (choice == "3") {
                saveData(backend, store);
                printPage("Goodbye", "Your data is saved for later Qt integration.");
                break;
            } else {
                cout << "Invalid choice.\n";
            }
        }

        return 0;
    } catch (const exception& error) {
        cerr << "Terminal demo failed: " << error.what() << '\n';
        return 1;
    }
}
