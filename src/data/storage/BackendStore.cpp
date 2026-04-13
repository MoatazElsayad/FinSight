#include "BackendStore.h"

#include "../json/TextCodec.h"
#include "../../core/models/Receipt.h"
#include "../../core/models/Report.h"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>

#include <sqlite3.h>

using namespace std;

namespace finsight::data::storage {

namespace {

using namespace finsight::core;

// Manages the SQLite connection lifetime for one save/load operation.
class SqliteConnection {
public:
    explicit SqliteConnection(const filesystem::path& path) {
        if (sqlite3_open(path.string().c_str(), &db_) != SQLITE_OK) {
            const string message = db_ == nullptr ? "Failed to open SQLite database."
                                                  : sqlite3_errmsg(db_);
            if (db_ != nullptr) {
                sqlite3_close(db_);
                db_ = nullptr;
            }
            throw runtime_error(message);
        }
    }

    ~SqliteConnection() {
        if (db_ != nullptr) {
            sqlite3_close(db_);
        }
    }

    sqlite3* get() const {
        return db_;
    }

private:
    sqlite3* db_ {nullptr};
};

// Executes one SQL statement without returning rows.
void exec(sqlite3* db, const string& sql) {
    char* errorMessage = nullptr;
    if (sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errorMessage) != SQLITE_OK) {
        const string message = errorMessage == nullptr ? "SQLite execution failed." : errorMessage;
        sqlite3_free(errorMessage);
        throw runtime_error(message);
    }
}

// Prepares one reusable SQLite statement.
sqlite3_stmt* prepare(sqlite3* db, const string& sql) {
    sqlite3_stmt* statement = nullptr;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &statement, nullptr) != SQLITE_OK) {
        throw runtime_error(sqlite3_errmsg(db));
    }
    return statement;
}

// Finalizes a prepared statement.
void finalize(sqlite3_stmt* statement) {
    if (statement != nullptr) {
        sqlite3_finalize(statement);
    }
}

// Resets a prepared statement after one execution.
void reset(sqlite3_stmt* statement) {
    sqlite3_reset(statement);
    sqlite3_clear_bindings(statement);
}

// Copies UTF-8 text from a SQLite column, or an empty string if NULL.
string columnText(sqlite3_stmt* statement, int index) {
    if (sqlite3_column_type(statement, index) == SQLITE_NULL) {
        return {};
    }
    const unsigned char* text = sqlite3_column_text(statement, index);
    if (text == nullptr) {
        return {};
    }
    return string(reinterpret_cast<const char*>(text));
}

// Steps a prepared statement and throws on failure.
void stepDone(sqlite3* db, sqlite3_stmt* statement) {
    if (sqlite3_step(statement) != SQLITE_DONE) {
        throw runtime_error(sqlite3_errmsg(db));
    }
}

// Binds a string value to a statement parameter.
void bindText(sqlite3* db, sqlite3_stmt* statement, int index, const string& value) {
    if (sqlite3_bind_text(statement, index, value.c_str(), -1, SQLITE_TRANSIENT) != SQLITE_OK) {
        throw runtime_error(sqlite3_errmsg(db));
    }
}

// Binds a double value to a statement parameter.
void bindDouble(sqlite3* db, sqlite3_stmt* statement, int index, double value) {
    if (sqlite3_bind_double(statement, index, value) != SQLITE_OK) {
        throw runtime_error(sqlite3_errmsg(db));
    }
}

// Binds an integer value to a statement parameter.
void bindInt(sqlite3* db, sqlite3_stmt* statement, int index, int value) {
    if (sqlite3_bind_int(statement, index, value) != SQLITE_OK) {
        throw runtime_error(sqlite3_errmsg(db));
    }
}

// Converts a boolean into a compact persisted string.
string boolToString(bool value) {
    return value ? "1" : "0";
}

// Restores a boolean value from persisted text.
bool toBool(const string& value) {
    return value == "1" || value == "true";
}

// Converts a category enum into storage text.
string toString(models::CategoryKind kind) {
    switch (kind) {
    case models::CategoryKind::Income:
        return "income";
    case models::CategoryKind::Expense:
        return "expense";
    case models::CategoryKind::Savings:
        return "savings";
    case models::CategoryKind::Investment:
        return "investment";
    }
    return "expense";
}

// Restores a category enum from storage text.
models::CategoryKind toCategoryKind(const string& value) {
    if (value == "income") return models::CategoryKind::Income;
    if (value == "savings") return models::CategoryKind::Savings;
    if (value == "investment") return models::CategoryKind::Investment;
    return models::CategoryKind::Expense;
}

// Converts a transaction enum into storage text.
string toString(models::TransactionType type) {
    return type == models::TransactionType::Income ? "income" : "expense";
}

// Restores a transaction enum from storage text.
models::TransactionType toTransactionType(const string& value) {
    return value == "income" ? models::TransactionType::Income : models::TransactionType::Expense;
}

// Converts a savings-entry enum into storage text.
string toString(models::SavingsEntryType type) {
    return type == models::SavingsEntryType::Deposit ? "deposit" : "withdrawal";
}

// Restores a savings-entry enum from storage text.
models::SavingsEntryType toSavingsEntryType(const string& value) {
    return value == "withdrawal" ? models::SavingsEntryType::Withdrawal : models::SavingsEntryType::Deposit;
}

// Converts an investment enum into storage text.
string toString(models::InvestmentType type) {
    switch (type) {
    case models::InvestmentType::Gold:
        return "gold";
    case models::InvestmentType::Silver:
        return "silver";
    case models::InvestmentType::Currency:
        return "currency";
    case models::InvestmentType::Stock:
        return "stock";
    case models::InvestmentType::Other:
        return "other";
    }
    return "other";
}

// Restores an investment enum from storage text.
models::InvestmentType toInvestmentType(const string& value) {
    if (value == "gold") return models::InvestmentType::Gold;
    if (value == "silver") return models::InvestmentType::Silver;
    if (value == "currency") return models::InvestmentType::Currency;
    if (value == "stock") return models::InvestmentType::Stock;
    return models::InvestmentType::Other;
}

// Converts a receipt status enum into storage text.
string toString(models::ReceiptStatus status) {
    switch (status) {
    case models::ReceiptStatus::Uploaded:
        return "uploaded";
    case models::ReceiptStatus::Parsed:
        return "parsed";
    case models::ReceiptStatus::Confirmed:
        return "confirmed";
    }
    return "uploaded";
}

// Restores a receipt status enum from storage text.
models::ReceiptStatus toReceiptStatus(const string& value) {
    if (value == "parsed") return models::ReceiptStatus::Parsed;
    if (value == "confirmed") return models::ReceiptStatus::Confirmed;
    return models::ReceiptStatus::Uploaded;
}

// Converts an optional date into persisted text.
string optionalDate(const optional<models::Date>& value) {
    return value ? value->toString() : "";
}

// Converts an optional amount into persisted text.
string optionalAmount(const optional<double>& value) {
    return value ? to_string(*value) : "";
}

// Parses an optional date from stored text.
optional<models::Date> parseOptionalDate(const string& value) {
    if (value.empty()) {
        return nullopt;
    }
    return models::Date::fromString(value);
}

// Parses an optional amount from stored text.
optional<double> parseOptionalAmount(const string& value) {
    if (value.empty()) {
        return nullopt;
    }
    return stod(value);
}

// Escapes raw text before it is written inside JSON string values.
string jsonEscape(const string& value) {
    string escaped;
    for (char ch : value) {
        switch (ch) {
        case '\\':
            escaped += "\\\\";
            break;
        case '"':
            escaped += "\\\"";
            break;
        case '\n':
            escaped += "\\n";
            break;
        case '\r':
            escaped += "\\r";
            break;
        case '\t':
            escaped += "\\t";
            break;
        default:
            escaped.push_back(ch);
            break;
        }
    }
    return escaped;
}

// Restores escaped JSON characters inside string values.
string jsonUnescape(const string& value) {
    string unescaped;
    bool escaped = false;
    for (char ch : value) {
        if (escaped) {
            switch (ch) {
            case 'n':
                unescaped.push_back('\n');
                break;
            case 'r':
                unescaped.push_back('\r');
                break;
            case 't':
                unescaped.push_back('\t');
                break;
            default:
                unescaped.push_back(ch);
                break;
            }
            escaped = false;
            continue;
        }
        if (ch == '\\') {
            escaped = true;
            continue;
        }
        unescaped.push_back(ch);
    }
    return unescaped;
}

// Writes one named JSON array that stores encoded row strings.
void appendStringArray(ostringstream& jsonOut, const string& key, const vector<string>& values, bool trailingComma) {
    jsonOut << "  \"" << key << "\":[";
    for (size_t index = 0; index < values.size(); ++index) {
        if (index > 0) {
            jsonOut << ",";
        }
        jsonOut << "\"" << jsonEscape(values[index]) << "\"";
    }
    jsonOut << "]";
    if (trailingComma) {
        jsonOut << ",";
    }
    jsonOut << "\n";
}

// Parses one JSON string array written by appendStringArray.
vector<string> parseStringArray(const string& content, const string& key) {
    const string marker = "\"" + key + "\":[";
    const auto markerPosition = content.find(marker);
    if (markerPosition == string::npos) {
        return {};
    }

    vector<string> values;
    size_t index = markerPosition + marker.size();
    while (index < content.size()) {
        while (index < content.size() && isspace(static_cast<unsigned char>(content[index]))) {
            ++index;
        }
        if (index >= content.size() || content[index] == ']') {
            break;
        }
        if (content[index] != '"') {
            break;
        }

        ++index;
        string value;
        bool escaped = false;
        while (index < content.size()) {
            const char ch = content[index++];
            if (escaped) {
                value.push_back('\\');
                value.push_back(ch);
                escaped = false;
                continue;
            }
            if (ch == '\\') {
                escaped = true;
                continue;
            }
            if (ch == '"') {
                break;
            }
            value.push_back(ch);
        }
        values.push_back(jsonUnescape(value));

        while (index < content.size() && isspace(static_cast<unsigned char>(content[index]))) {
            ++index;
        }
        if (index < content.size() && content[index] == ',') {
            ++index;
        }
    }
    return values;
}

// Writes the flexible side data to one JSON sidecar file.
void writeSidecarJson(const filesystem::path& path,
                      const vector<string>& receipts,
                      const vector<string>& parsedReceipts,
                      const vector<string>& pantryItems,
                      const vector<string>& shoppingItems) {
    ofstream output(path, ios::trunc);
    if (!output) {
        throw runtime_error("Failed to open JSON sidecar for writing: " + path.string());
    }

    ostringstream jsonOut;
    jsonOut << "{\n";
    jsonOut << "  \"format\":\"finsight-sidecar-v1\",\n";
    appendStringArray(jsonOut, "receipts", receipts, true);
    appendStringArray(jsonOut, "parsedReceipts", parsedReceipts, true);
    appendStringArray(jsonOut, "pantryItems", pantryItems, true);
    appendStringArray(jsonOut, "shoppingItems", shoppingItems, false);
    jsonOut << "}\n";
    output << jsonOut.str();
}

// Reads the full JSON sidecar file if it exists.
string readSidecarJson(const filesystem::path& path) {
    ifstream input(path);
    if (!input) {
        return {};
    }
    ostringstream content;
    content << input.rdbuf();
    return content.str();
}

// Creates the SQLite schema used for the structured app data.
void ensureSchema(sqlite3* db) {
    exec(db, R"sql(
        CREATE TABLE IF NOT EXISTS users (
            id TEXT PRIMARY KEY,
            full_name TEXT NOT NULL,
            email TEXT NOT NULL,
            phone TEXT NOT NULL,
            gender TEXT NOT NULL,
            password_hash TEXT NOT NULL,
            created_at TEXT NOT NULL
        );
        CREATE TABLE IF NOT EXISTS categories (
            id TEXT PRIMARY KEY,
            user_id TEXT NOT NULL,
            name TEXT NOT NULL,
            icon TEXT NOT NULL,
            kind TEXT NOT NULL,
            built_in INTEGER NOT NULL,
            archived INTEGER NOT NULL
        );
        CREATE TABLE IF NOT EXISTS transactions (
            id TEXT PRIMARY KEY,
            user_id TEXT NOT NULL,
            title TEXT NOT NULL,
            description TEXT NOT NULL,
            category_id TEXT NOT NULL,
            type TEXT NOT NULL,
            amount REAL NOT NULL,
            date TEXT NOT NULL,
            merchant TEXT NOT NULL,
            tags TEXT NOT NULL
        );
        CREATE TABLE IF NOT EXISTS budgets (
            id TEXT PRIMARY KEY,
            user_id TEXT NOT NULL,
            category_id TEXT NOT NULL,
            year INTEGER NOT NULL,
            month INTEGER NOT NULL,
            limit_amount REAL NOT NULL
        );
        CREATE TABLE IF NOT EXISTS savings_entries (
            id TEXT PRIMARY KEY,
            user_id TEXT NOT NULL,
            type TEXT NOT NULL,
            amount REAL NOT NULL,
            date TEXT NOT NULL,
            note TEXT NOT NULL
        );
        CREATE TABLE IF NOT EXISTS savings_goals (
            id TEXT PRIMARY KEY,
            user_id TEXT NOT NULL,
            monthly_target REAL NOT NULL,
            long_term_target REAL NOT NULL,
            target_date TEXT NOT NULL
        );
        CREATE TABLE IF NOT EXISTS investments (
            id TEXT PRIMARY KEY,
            user_id TEXT NOT NULL,
            asset_name TEXT NOT NULL,
            symbol TEXT NOT NULL,
            type TEXT NOT NULL,
            quantity REAL NOT NULL,
            buy_rate REAL NOT NULL,
            current_rate REAL NOT NULL,
            purchase_date TEXT NOT NULL
        );
        CREATE TABLE IF NOT EXISTS goals (
            id TEXT PRIMARY KEY,
            user_id TEXT NOT NULL,
            title TEXT NOT NULL,
            description TEXT NOT NULL,
            target_amount REAL NOT NULL,
            current_amount REAL NOT NULL,
            target_date TEXT NOT NULL,
            completed INTEGER NOT NULL
        );
        CREATE TABLE IF NOT EXISTS sessions (
            token TEXT PRIMARY KEY,
            user_id TEXT NOT NULL,
            issued_on TEXT NOT NULL,
            active INTEGER NOT NULL
        );
    )sql");
}

// Clears existing rows before a full save.
void clearTables(sqlite3* db) {
    exec(db, R"sql(
        DELETE FROM users;
        DELETE FROM categories;
        DELETE FROM transactions;
        DELETE FROM budgets;
        DELETE FROM savings_entries;
        DELETE FROM savings_goals;
        DELETE FROM investments;
        DELETE FROM goals;
        DELETE FROM sessions;
    )sql");
}

// Removes the old file-based artifacts left from the previous persistence version.
void cleanupLegacyFiles(const filesystem::path& directory) {
    const vector<filesystem::path> legacyFiles {
        directory / "users.tsv",
        directory / "categories.tsv",
        directory / "transactions.tsv",
        directory / "budgets.tsv",
        directory / "savings_entries.tsv",
        directory / "savings_goals.tsv",
        directory / "investments.tsv",
        directory / "goals.tsv",
        directory / "sessions.tsv",
        directory / "receipts.tsv",
        directory / "parsed_receipts.tsv",
        directory / "pantry.tsv",
        directory / "shopping.tsv",
    };

    for (const auto& path : legacyFiles) {
        error_code error;
        filesystem::remove(path, error);
    }
}

}  // namespace

// Returns the SQLite database path for one persistence directory.
filesystem::path BackendStore::databasePath(const filesystem::path& directory) const {
    return directory / "finsight.db";
}

// Returns the JSON sidecar path for one persistence directory.
filesystem::path BackendStore::sidecarPath(const filesystem::path& directory) const {
    return directory / "sidecar.json";
}

// Saves the backend using SQLite for core data and JSON for flexible side data.
void BackendStore::save(const core::managers::FinanceTrackerBackend& backend,
                        const filesystem::path& directory) const {
    filesystem::create_directories(directory);
    cleanupLegacyFiles(directory);

    SqliteConnection connection(databasePath(directory));
    sqlite3* db = connection.get();
    ensureSchema(db);
    exec(db, "BEGIN IMMEDIATE TRANSACTION;");
    try {
        clearTables(db);

        sqlite3_stmt* statement = nullptr;

        statement = prepare(db, "INSERT INTO users VALUES (?, ?, ?, ?, ?, ?, ?);");
        for (const auto& user : backend.auth().listUsers()) {
            bindText(db, statement, 1, user.id);
            bindText(db, statement, 2, user.fullName);
            bindText(db, statement, 3, user.email);
            bindText(db, statement, 4, user.phone);
            bindText(db, statement, 5, user.gender);
            bindText(db, statement, 6, user.passwordHash);
            bindText(db, statement, 7, user.createdAt.toString());
            stepDone(db, statement);
            reset(statement);
        }
        finalize(statement);

        statement = prepare(db, "INSERT INTO categories VALUES (?, ?, ?, ?, ?, ?, ?);");
        for (const auto& category : backend.transactions().getCategories()) {
            bindText(db, statement, 1, category.id);
            bindText(db, statement, 2, category.userId);
            bindText(db, statement, 3, category.name);
            bindText(db, statement, 4, category.icon);
            bindText(db, statement, 5, toString(category.kind));
            bindInt(db, statement, 6, category.builtIn ? 1 : 0);
            bindInt(db, statement, 7, category.archived ? 1 : 0);
            stepDone(db, statement);
            reset(statement);
        }
        finalize(statement);

        statement = prepare(db, "INSERT INTO transactions VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?);");
        for (const auto& transaction : backend.transactions().allTransactions()) {
            bindText(db, statement, 1, transaction.id);
            bindText(db, statement, 2, transaction.userId);
            bindText(db, statement, 3, transaction.title);
            bindText(db, statement, 4, transaction.description);
            bindText(db, statement, 5, transaction.categoryId);
            bindText(db, statement, 6, toString(transaction.type));
            bindDouble(db, statement, 7, transaction.amount);
            bindText(db, statement, 8, transaction.date.toString());
            bindText(db, statement, 9, transaction.merchant);
            bindText(db, statement, 10, json::encodeList(transaction.tags));
            stepDone(db, statement);
            reset(statement);
        }
        finalize(statement);

        statement = prepare(db, "INSERT INTO budgets VALUES (?, ?, ?, ?, ?, ?);");
        for (const auto& budget : backend.budgets().allBudgets()) {
            bindText(db, statement, 1, budget.id);
            bindText(db, statement, 2, budget.userId);
            bindText(db, statement, 3, budget.categoryId);
            bindInt(db, statement, 4, budget.period.year);
            bindInt(db, statement, 5, budget.period.month);
            bindDouble(db, statement, 6, budget.limit);
            stepDone(db, statement);
            reset(statement);
        }
        finalize(statement);

        statement = prepare(db, "INSERT INTO savings_entries VALUES (?, ?, ?, ?, ?, ?);");
        for (const auto& entry : backend.savings().allEntries()) {
            bindText(db, statement, 1, entry.id);
            bindText(db, statement, 2, entry.userId);
            bindText(db, statement, 3, toString(entry.type));
            bindDouble(db, statement, 4, entry.amount);
            bindText(db, statement, 5, entry.date.toString());
            bindText(db, statement, 6, entry.note);
            stepDone(db, statement);
            reset(statement);
        }
        finalize(statement);

        statement = prepare(db, "INSERT INTO savings_goals VALUES (?, ?, ?, ?, ?);");
        for (const auto& goal : backend.savings().allGoals()) {
            bindText(db, statement, 1, goal.id);
            bindText(db, statement, 2, goal.userId);
            bindDouble(db, statement, 3, goal.monthlyTarget);
            bindDouble(db, statement, 4, goal.longTermTarget);
            bindText(db, statement, 5, goal.targetDate.toString());
            stepDone(db, statement);
            reset(statement);
        }
        finalize(statement);

        statement = prepare(db, "INSERT INTO investments VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?);");
        for (const auto& investment : backend.savings().allInvestments()) {
            bindText(db, statement, 1, investment.id);
            bindText(db, statement, 2, investment.userId);
            bindText(db, statement, 3, investment.assetName);
            bindText(db, statement, 4, investment.symbol);
            bindText(db, statement, 5, toString(investment.type));
            bindDouble(db, statement, 6, investment.quantity);
            bindDouble(db, statement, 7, investment.buyRate);
            bindDouble(db, statement, 8, investment.currentRate);
            bindText(db, statement, 9, investment.purchaseDate.toString());
            stepDone(db, statement);
            reset(statement);
        }
        finalize(statement);

        statement = prepare(db, "INSERT INTO goals VALUES (?, ?, ?, ?, ?, ?, ?, ?);");
        for (const auto& goal : backend.goals().allGoals()) {
            bindText(db, statement, 1, goal.id);
            bindText(db, statement, 2, goal.userId);
            bindText(db, statement, 3, goal.title);
            bindText(db, statement, 4, goal.description);
            bindDouble(db, statement, 5, goal.targetAmount);
            bindDouble(db, statement, 6, goal.currentAmount);
            bindText(db, statement, 7, goal.targetDate.toString());
            bindInt(db, statement, 8, goal.completed ? 1 : 0);
            stepDone(db, statement);
            reset(statement);
        }
        finalize(statement);

        statement = prepare(db, "INSERT INTO sessions VALUES (?, ?, ?, ?);");
        for (const auto& session : backend.sessions().allSessions()) {
            bindText(db, statement, 1, session.token);
            bindText(db, statement, 2, session.userId);
            bindText(db, statement, 3, session.issuedOn.toString());
            bindInt(db, statement, 4, session.active ? 1 : 0);
            stepDone(db, statement);
            reset(statement);
        }
        finalize(statement);

        exec(db, "COMMIT;");
    } catch (...) {
        exec(db, "ROLLBACK;");
        throw;
    }

    vector<string> receipts;
    for (const auto& receipt : backend.receipts().allReceipts()) {
        receipts.push_back(json::encodeRow({
            receipt.id, receipt.userId, receipt.fileName, receipt.rawText,
            toString(receipt.status), receipt.uploadedAt.toString(),
        }));
    }

    vector<string> parsedReceipts;
    for (const auto& parsed : backend.receipts().allParsedReceipts()) {
        parsedReceipts.push_back(json::encodeRow({
            parsed.receiptId, parsed.merchant, optionalDate(parsed.transactionDate),
            optionalAmount(parsed.amount), parsed.suggestedCategoryId, parsed.confidenceNotes,
            json::encodeList(parsed.extractedLines),
        }));
    }

    vector<string> pantryItems;
    for (const auto& item : backend.shopping().allPantryItems()) {
        pantryItems.push_back(json::encodeRow({
            item.id, item.userId, item.name, item.unit, to_string(item.quantity),
            to_string(item.lowStockThreshold), item.updatedAt.toString(),
        }));
    }

    vector<string> shoppingItems;
    for (const auto& item : backend.shopping().allShoppingItems()) {
        shoppingItems.push_back(json::encodeRow({
            item.id, item.userId, item.name, item.category, to_string(item.plannedQuantity),
            item.unit, boolToString(item.purchased), item.createdAt.toString(),
        }));
    }

    writeSidecarJson(sidecarPath(directory), receipts, parsedReceipts, pantryItems, shoppingItems);
}

// Loads the backend using SQLite for core data and JSON for flexible side data.
void BackendStore::load(core::managers::FinanceTrackerBackend& backend,
                        const filesystem::path& directory) const {
    const auto dbFile = databasePath(directory);
    if (filesystem::exists(dbFile)) {
        SqliteConnection connection(dbFile);
        sqlite3* db = connection.get();
        ensureSchema(db);

        vector<core::models::User> users;
        sqlite3_stmt* statement = prepare(
            db,
            "SELECT id, full_name, email, phone, gender, password_hash, created_at FROM users;");
        while (sqlite3_step(statement) == SQLITE_ROW) {
            const string createdAtText = columnText(statement, 6);
            users.push_back(core::models::User {
                .id = columnText(statement, 0),
                .fullName = columnText(statement, 1),
                .email = columnText(statement, 2),
                .phone = columnText(statement, 3),
                .gender = columnText(statement, 4),
                .passwordHash = columnText(statement, 5),
                .createdAt = createdAtText.empty()
                                 ? core::models::Date {1970, 1, 1}
                                 : core::models::Date::fromString(createdAtText),
            });
        }
        finalize(statement);
        backend.auth().loadUsers(move(users));

        vector<core::models::Category> categories;
        statement = prepare(db, "SELECT id, user_id, name, icon, kind, built_in, archived FROM categories;");
        while (sqlite3_step(statement) == SQLITE_ROW) {
            categories.push_back(core::models::Category {
                .id = columnText(statement, 0),
                .userId = columnText(statement, 1),
                .name = columnText(statement, 2),
                .icon = columnText(statement, 3),
                .kind = toCategoryKind(columnText(statement, 4)),
                .builtIn = sqlite3_column_int(statement, 5) != 0,
                .archived = sqlite3_column_int(statement, 6) != 0,
            });
        }
        finalize(statement);

        vector<core::models::Transaction> transactions;
        statement = prepare(db, "SELECT id, user_id, title, description, category_id, type, amount, date, merchant, tags FROM transactions;");
        while (sqlite3_step(statement) == SQLITE_ROW) {
            const string dateText = columnText(statement, 7);
            transactions.push_back(core::models::Transaction {
                .id = columnText(statement, 0),
                .userId = columnText(statement, 1),
                .title = columnText(statement, 2),
                .description = columnText(statement, 3),
                .categoryId = columnText(statement, 4),
                .type = toTransactionType(columnText(statement, 5)),
                .amount = sqlite3_column_double(statement, 6),
                .date = dateText.empty() ? core::models::Date {1970, 1, 1}
                                         : core::models::Date::fromString(dateText),
                .merchant = columnText(statement, 8),
                .tags = json::decodeList(columnText(statement, 9)),
            });
        }
        finalize(statement);
        if (categories.empty()) {
            categories = backend.transactions().getCategories();
        }
        backend.transactions().loadState(move(categories), move(transactions));

        vector<core::models::Budget> budgets;
        statement = prepare(db, "SELECT id, user_id, category_id, year, month, limit_amount FROM budgets;");
        while (sqlite3_step(statement) == SQLITE_ROW) {
            budgets.push_back(core::models::Budget {
                .id = columnText(statement, 0),
                .userId = columnText(statement, 1),
                .categoryId = columnText(statement, 2),
                .period = core::models::YearMonth {sqlite3_column_int(statement, 3), sqlite3_column_int(statement, 4)},
                .limit = sqlite3_column_double(statement, 5),
            });
        }
        finalize(statement);
        backend.budgets().loadBudgets(move(budgets));

        vector<core::models::SavingsEntry> savingsEntries;
        statement = prepare(db, "SELECT id, user_id, type, amount, date, note FROM savings_entries;");
        while (sqlite3_step(statement) == SQLITE_ROW) {
            const string entryDate = columnText(statement, 4);
            savingsEntries.push_back(core::models::SavingsEntry {
                .id = columnText(statement, 0),
                .userId = columnText(statement, 1),
                .type = toSavingsEntryType(columnText(statement, 2)),
                .amount = sqlite3_column_double(statement, 3),
                .date = entryDate.empty() ? core::models::Date {1970, 1, 1}
                                          : core::models::Date::fromString(entryDate),
                .note = columnText(statement, 5),
            });
        }
        finalize(statement);

        vector<core::models::SavingsGoal> savingsGoals;
        statement = prepare(db, "SELECT id, user_id, monthly_target, long_term_target, target_date FROM savings_goals;");
        while (sqlite3_step(statement) == SQLITE_ROW) {
            const string goalDate = columnText(statement, 4);
            savingsGoals.push_back(core::models::SavingsGoal {
                .id = columnText(statement, 0),
                .userId = columnText(statement, 1),
                .monthlyTarget = sqlite3_column_double(statement, 2),
                .longTermTarget = sqlite3_column_double(statement, 3),
                .targetDate = goalDate.empty() ? core::models::Date {1970, 1, 1}
                                               : core::models::Date::fromString(goalDate),
            });
        }
        finalize(statement);

        vector<core::models::Investment> investments;
        statement = prepare(db, "SELECT id, user_id, asset_name, symbol, type, quantity, buy_rate, current_rate, purchase_date FROM investments;");
        while (sqlite3_step(statement) == SQLITE_ROW) {
            const string purchaseDate = columnText(statement, 8);
            investments.push_back(core::models::Investment {
                .id = columnText(statement, 0),
                .userId = columnText(statement, 1),
                .assetName = columnText(statement, 2),
                .symbol = columnText(statement, 3),
                .type = toInvestmentType(columnText(statement, 4)),
                .quantity = sqlite3_column_double(statement, 5),
                .buyRate = sqlite3_column_double(statement, 6),
                .currentRate = sqlite3_column_double(statement, 7),
                .purchaseDate = purchaseDate.empty() ? core::models::Date {1970, 1, 1}
                                                     : core::models::Date::fromString(purchaseDate),
            });
        }
        finalize(statement);
        backend.savings().loadState(move(savingsEntries), move(investments), move(savingsGoals));

        vector<core::models::Goal> goals;
        statement = prepare(db, "SELECT id, user_id, title, description, target_amount, current_amount, target_date, completed FROM goals;");
        while (sqlite3_step(statement) == SQLITE_ROW) {
            const string targetDateText = columnText(statement, 6);
            goals.push_back(core::models::Goal {
                .id = columnText(statement, 0),
                .userId = columnText(statement, 1),
                .title = columnText(statement, 2),
                .description = columnText(statement, 3),
                .targetAmount = sqlite3_column_double(statement, 4),
                .currentAmount = sqlite3_column_double(statement, 5),
                .targetDate = targetDateText.empty() ? core::models::Date {1970, 1, 1}
                                                     : core::models::Date::fromString(targetDateText),
                .completed = sqlite3_column_int(statement, 7) != 0,
            });
        }
        finalize(statement);
        backend.goals().loadGoals(move(goals));

        vector<core::models::Session> sessions;
        statement = prepare(db, "SELECT token, user_id, issued_on, active FROM sessions;");
        while (sqlite3_step(statement) == SQLITE_ROW) {
            const string issued = columnText(statement, 2);
            sessions.push_back(core::models::Session {
                .token = columnText(statement, 0),
                .userId = columnText(statement, 1),
                .issuedOn = issued.empty() ? core::models::Date {1970, 1, 1}
                                           : core::models::Date::fromString(issued),
                .active = sqlite3_column_int(statement, 3) != 0,
            });
        }
        finalize(statement);
        backend.sessions().loadSessions(move(sessions));
    }

    const auto sidecarContent = readSidecarJson(sidecarPath(directory));
    if (sidecarContent.empty()) {
        return;
    }

    vector<core::models::ReceiptDocument> receipts;
    for (const auto& row : parseStringArray(sidecarContent, "receipts")) {
        const auto fields = json::decodeRow(row);
        if (fields.size() < 6) continue;
        receipts.push_back(core::models::ReceiptDocument {
            .id = fields[0],
            .userId = fields[1],
            .fileName = fields[2],
            .rawText = fields[3],
            .status = toReceiptStatus(fields[4]),
            .uploadedAt = core::models::Date::fromString(fields[5]),
        });
    }

    vector<core::models::ReceiptParseResult> parsedReceipts;
    for (const auto& row : parseStringArray(sidecarContent, "parsedReceipts")) {
        const auto fields = json::decodeRow(row);
        if (fields.size() < 7) continue;
        parsedReceipts.push_back(core::models::ReceiptParseResult {
            .receiptId = fields[0],
            .merchant = fields[1],
            .transactionDate = parseOptionalDate(fields[2]),
            .amount = parseOptionalAmount(fields[3]),
            .suggestedCategoryId = fields[4],
            .confidenceNotes = fields[5],
            .extractedLines = json::decodeList(fields[6]),
        });
    }
    backend.receipts().loadState(move(receipts), move(parsedReceipts));

    vector<core::models::PantryItem> pantryItems;
    for (const auto& row : parseStringArray(sidecarContent, "pantryItems")) {
        const auto fields = json::decodeRow(row);
        if (fields.size() < 7) continue;
        pantryItems.push_back(core::models::PantryItem {
            .id = fields[0],
            .userId = fields[1],
            .name = fields[2],
            .unit = fields[3],
            .quantity = stod(fields[4]),
            .lowStockThreshold = stod(fields[5]),
            .updatedAt = core::models::Date::fromString(fields[6]),
        });
    }

    vector<core::models::ShoppingItem> shoppingItems;
    for (const auto& row : parseStringArray(sidecarContent, "shoppingItems")) {
        const auto fields = json::decodeRow(row);
        if (fields.size() < 8) continue;
        shoppingItems.push_back(core::models::ShoppingItem {
            .id = fields[0],
            .userId = fields[1],
            .name = fields[2],
            .category = fields[3],
            .plannedQuantity = stod(fields[4]),
            .unit = fields[5],
            .purchased = toBool(fields[6]),
            .createdAt = core::models::Date::fromString(fields[7]),
        });
    }
    backend.shopping().loadState(move(pantryItems), move(shoppingItems));
}

}  // namespace finsight::data::storage
