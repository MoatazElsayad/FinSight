#include "BackendStore.h"

#include "../json/TextCodec.h"
#include "../../core/models/Receipt.h"
#include "../../core/models/Report.h"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>

namespace finsight::data::storage {

namespace {

using namespace finsight::core;

std::string boolToString(bool value) {
    return value ? "1" : "0";
}

bool toBool(const std::string& value) {
    return value == "1" || value == "true";
}

std::string toString(models::CategoryKind kind) {
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

models::CategoryKind toCategoryKind(const std::string& value) {
    if (value == "income") return models::CategoryKind::Income;
    if (value == "savings") return models::CategoryKind::Savings;
    if (value == "investment") return models::CategoryKind::Investment;
    return models::CategoryKind::Expense;
}

std::string toString(models::TransactionType type) {
    return type == models::TransactionType::Income ? "income" : "expense";
}

models::TransactionType toTransactionType(const std::string& value) {
    return value == "income" ? models::TransactionType::Income : models::TransactionType::Expense;
}

std::string toString(models::SavingsEntryType type) {
    return type == models::SavingsEntryType::Deposit ? "deposit" : "withdrawal";
}

models::SavingsEntryType toSavingsEntryType(const std::string& value) {
    return value == "withdrawal" ? models::SavingsEntryType::Withdrawal : models::SavingsEntryType::Deposit;
}

std::string toString(models::InvestmentType type) {
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

models::InvestmentType toInvestmentType(const std::string& value) {
    if (value == "gold") return models::InvestmentType::Gold;
    if (value == "silver") return models::InvestmentType::Silver;
    if (value == "currency") return models::InvestmentType::Currency;
    if (value == "stock") return models::InvestmentType::Stock;
    return models::InvestmentType::Other;
}

std::string toString(models::ReceiptStatus status) {
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

models::ReceiptStatus toReceiptStatus(const std::string& value) {
    if (value == "parsed") return models::ReceiptStatus::Parsed;
    if (value == "confirmed") return models::ReceiptStatus::Confirmed;
    return models::ReceiptStatus::Uploaded;
}

void writeLines(const std::filesystem::path& path, const std::vector<std::string>& lines) {
    std::ofstream output(path, std::ios::trunc);
    if (!output) {
        throw std::runtime_error("Failed to open file for writing: " + path.string());
    }
    for (const auto& line : lines) {
        output << line << '\n';
    }
}

std::vector<std::string> readLines(const std::filesystem::path& path) {
    std::vector<std::string> lines;
    std::ifstream input(path);
    if (!input) {
        return lines;
    }

    std::string line;
    while (std::getline(input, line)) {
        if (!line.empty()) {
            lines.push_back(line);
        }
    }
    return lines;
}

double toDouble(const std::string& value) {
    return value.empty() ? 0.0 : std::stod(value);
}

int toInt(const std::string& value) {
    return value.empty() ? 0 : std::stoi(value);
}

std::string optionalDate(const std::optional<models::Date>& value) {
    return value ? value->toString() : "";
}

std::string optionalAmount(const std::optional<double>& value) {
    return value ? std::to_string(*value) : "";
}

std::optional<models::Date> parseOptionalDate(const std::string& value) {
    if (value.empty()) {
        return std::nullopt;
    }
    return models::Date::fromString(value);
}

std::optional<double> parseOptionalAmount(const std::string& value) {
    if (value.empty()) {
        return std::nullopt;
    }
    return std::stod(value);
}

}  // namespace

void BackendStore::save(const core::managers::FinanceTrackerBackend& backend,
                        const std::filesystem::path& directory) const {
    std::filesystem::create_directories(directory);

    std::vector<std::string> users;
    for (const auto& user : backend.auth().listUsers()) {
        users.push_back(json::encodeRow({
            user.id, user.fullName, user.email, user.phone, user.gender, user.passwordHash, user.createdAt.toString()}));
    }
    writeLines(directory / "users.tsv", users);

    std::vector<std::string> categories;
    for (const auto& category : backend.transactions().getCategories()) {
        categories.push_back(json::encodeRow({
            category.id, category.userId, category.name, category.icon, toString(category.kind),
            boolToString(category.builtIn), boolToString(category.archived)}));
    }
    writeLines(directory / "categories.tsv", categories);

    std::vector<std::string> transactions;
    for (const auto& transaction : backend.transactions().allTransactions()) {
        transactions.push_back(json::encodeRow({
            transaction.id, transaction.userId, transaction.title, transaction.description, transaction.categoryId,
            toString(transaction.type), std::to_string(transaction.amount), transaction.date.toString(),
            transaction.merchant, json::encodeList(transaction.tags)}));
    }
    writeLines(directory / "transactions.tsv", transactions);

    std::vector<std::string> budgets;
    for (const auto& budget : backend.budgets().allBudgets()) {
        budgets.push_back(json::encodeRow({
            budget.id, budget.userId, budget.categoryId, std::to_string(budget.period.year),
            std::to_string(budget.period.month), std::to_string(budget.limit)}));
    }
    writeLines(directory / "budgets.tsv", budgets);

    std::vector<std::string> savingsEntries;
    for (const auto& entry : backend.savings().allEntries()) {
        savingsEntries.push_back(json::encodeRow({
            entry.id, entry.userId, toString(entry.type), std::to_string(entry.amount), entry.date.toString(), entry.note}));
    }
    writeLines(directory / "savings_entries.tsv", savingsEntries);

    std::vector<std::string> savingsGoals;
    for (const auto& goal : backend.savings().allGoals()) {
        savingsGoals.push_back(json::encodeRow({
            goal.id, goal.userId, std::to_string(goal.monthlyTarget),
            std::to_string(goal.longTermTarget), goal.targetDate.toString()}));
    }
    writeLines(directory / "savings_goals.tsv", savingsGoals);

    std::vector<std::string> investmentRows;
    for (const auto& investment : backend.savings().allInvestments()) {
        investmentRows.push_back(json::encodeRow({
            investment.id, investment.userId, investment.assetName, investment.symbol, toString(investment.type),
            std::to_string(investment.quantity), std::to_string(investment.buyRate),
            std::to_string(investment.currentRate), investment.purchaseDate.toString()}));
    }
    writeLines(directory / "investments.tsv", investmentRows);

    std::vector<std::string> goals;
    for (const auto& goal : backend.goals().allGoals()) {
        goals.push_back(json::encodeRow({
            goal.id, goal.userId, goal.title, goal.description, std::to_string(goal.targetAmount),
            std::to_string(goal.currentAmount), goal.targetDate.toString(), boolToString(goal.completed)}));
    }
    writeLines(directory / "goals.tsv", goals);

    std::vector<std::string> sessions;
    for (const auto& session : backend.sessions().allSessions()) {
        sessions.push_back(json::encodeRow({
            session.token, session.userId, session.issuedOn.toString(), boolToString(session.active)}));
    }
    writeLines(directory / "sessions.tsv", sessions);

    std::vector<std::string> receipts;
    for (const auto& receipt : backend.receipts().allReceipts()) {
        receipts.push_back(json::encodeRow({
            receipt.id, receipt.userId, receipt.fileName, receipt.rawText,
            toString(receipt.status), receipt.uploadedAt.toString()}));
    }
    writeLines(directory / "receipts.tsv", receipts);

    std::vector<std::string> parsedReceipts;
    for (const auto& parsed : backend.receipts().allParsedReceipts()) {
        parsedReceipts.push_back(json::encodeRow({
            parsed.receiptId, parsed.merchant, optionalDate(parsed.transactionDate),
            optionalAmount(parsed.amount), parsed.suggestedCategoryId, parsed.confidenceNotes,
            json::encodeList(parsed.extractedLines)}));
    }
    writeLines(directory / "parsed_receipts.tsv", parsedReceipts);

    std::vector<std::string> pantryItems;
    for (const auto& item : backend.shopping().allPantryItems()) {
        pantryItems.push_back(json::encodeRow({
            item.id, item.userId, item.name, item.unit, std::to_string(item.quantity),
            std::to_string(item.lowStockThreshold), item.updatedAt.toString()}));
    }
    writeLines(directory / "pantry.tsv", pantryItems);

    std::vector<std::string> shoppingItems;
    for (const auto& item : backend.shopping().allShoppingItems()) {
        shoppingItems.push_back(json::encodeRow({
            item.id, item.userId, item.name, item.category, std::to_string(item.plannedQuantity),
            item.unit, boolToString(item.purchased), item.createdAt.toString()}));
    }
    writeLines(directory / "shopping.tsv", shoppingItems);
}

void BackendStore::load(core::managers::FinanceTrackerBackend& backend,
                        const std::filesystem::path& directory) const {
    std::vector<core::models::User> users;
    for (const auto& line : readLines(directory / "users.tsv")) {
        const auto fields = json::decodeRow(line);
        if (fields.size() < 7) continue;
        users.push_back(core::models::User {
            .id = fields[0],
            .fullName = fields[1],
            .email = fields[2],
            .phone = fields[3],
            .gender = fields[4],
            .passwordHash = fields[5],
            .createdAt = core::models::Date::fromString(fields[6]),
        });
    }
    backend.auth().loadUsers(std::move(users));

    std::vector<core::models::Category> categories;
    for (const auto& line : readLines(directory / "categories.tsv")) {
        const auto fields = json::decodeRow(line);
        if (fields.size() < 7) continue;
        categories.push_back(core::models::Category {
            .id = fields[0],
            .userId = fields[1],
            .name = fields[2],
            .icon = fields[3],
            .kind = toCategoryKind(fields[4]),
            .builtIn = toBool(fields[5]),
            .archived = toBool(fields[6]),
        });
    }

    std::vector<core::models::Transaction> transactions;
    for (const auto& line : readLines(directory / "transactions.tsv")) {
        const auto fields = json::decodeRow(line);
        if (fields.size() < 10) continue;
        transactions.push_back(core::models::Transaction {
            .id = fields[0],
            .userId = fields[1],
            .title = fields[2],
            .description = fields[3],
            .categoryId = fields[4],
            .type = toTransactionType(fields[5]),
            .amount = toDouble(fields[6]),
            .date = core::models::Date::fromString(fields[7]),
            .merchant = fields[8],
            .tags = json::decodeList(fields[9]),
        });
    }
    if (!categories.empty() || !transactions.empty()) {
        backend.transactions().loadState(std::move(categories), std::move(transactions));
    }

    std::vector<core::models::Budget> budgets;
    for (const auto& line : readLines(directory / "budgets.tsv")) {
        const auto fields = json::decodeRow(line);
        if (fields.size() < 6) continue;
        budgets.push_back(core::models::Budget {
            .id = fields[0],
            .userId = fields[1],
            .categoryId = fields[2],
            .period = core::models::YearMonth {toInt(fields[3]), toInt(fields[4])},
            .limit = toDouble(fields[5]),
        });
    }
    backend.budgets().loadBudgets(std::move(budgets));

    std::vector<core::models::SavingsEntry> savingsEntries;
    for (const auto& line : readLines(directory / "savings_entries.tsv")) {
        const auto fields = json::decodeRow(line);
        if (fields.size() < 6) continue;
        savingsEntries.push_back(core::models::SavingsEntry {
            .id = fields[0],
            .userId = fields[1],
            .type = toSavingsEntryType(fields[2]),
            .amount = toDouble(fields[3]),
            .date = core::models::Date::fromString(fields[4]),
            .note = fields[5],
        });
    }

    std::vector<core::models::Investment> investments;
    for (const auto& line : readLines(directory / "investments.tsv")) {
        const auto fields = json::decodeRow(line);
        if (fields.size() < 9) continue;
        investments.push_back(core::models::Investment {
            .id = fields[0],
            .userId = fields[1],
            .assetName = fields[2],
            .symbol = fields[3],
            .type = toInvestmentType(fields[4]),
            .quantity = toDouble(fields[5]),
            .buyRate = toDouble(fields[6]),
            .currentRate = toDouble(fields[7]),
            .purchaseDate = core::models::Date::fromString(fields[8]),
        });
    }

    std::vector<core::models::SavingsGoal> savingsGoals;
    for (const auto& line : readLines(directory / "savings_goals.tsv")) {
        const auto fields = json::decodeRow(line);
        if (fields.size() < 5) continue;
        savingsGoals.push_back(core::models::SavingsGoal {
            .id = fields[0],
            .userId = fields[1],
            .monthlyTarget = toDouble(fields[2]),
            .longTermTarget = toDouble(fields[3]),
            .targetDate = core::models::Date::fromString(fields[4]),
        });
    }
    backend.savings().loadState(std::move(savingsEntries), std::move(investments), std::move(savingsGoals));

    std::vector<core::models::Goal> goals;
    for (const auto& line : readLines(directory / "goals.tsv")) {
        const auto fields = json::decodeRow(line);
        if (fields.size() < 8) continue;
        goals.push_back(core::models::Goal {
            .id = fields[0],
            .userId = fields[1],
            .title = fields[2],
            .description = fields[3],
            .targetAmount = toDouble(fields[4]),
            .currentAmount = toDouble(fields[5]),
            .targetDate = core::models::Date::fromString(fields[6]),
            .completed = toBool(fields[7]),
        });
    }
    backend.goals().loadGoals(std::move(goals));

    std::vector<core::models::Session> sessions;
    for (const auto& line : readLines(directory / "sessions.tsv")) {
        const auto fields = json::decodeRow(line);
        if (fields.size() < 4) continue;
        sessions.push_back(core::models::Session {
            .token = fields[0],
            .userId = fields[1],
            .issuedOn = core::models::Date::fromString(fields[2]),
            .active = toBool(fields[3]),
        });
    }
    backend.sessions().loadSessions(std::move(sessions));

    std::vector<core::models::ReceiptDocument> receipts;
    for (const auto& line : readLines(directory / "receipts.tsv")) {
        const auto fields = json::decodeRow(line);
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

    std::vector<core::models::ReceiptParseResult> parsedReceipts;
    for (const auto& line : readLines(directory / "parsed_receipts.tsv")) {
        const auto fields = json::decodeRow(line);
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
    backend.receipts().loadState(std::move(receipts), std::move(parsedReceipts));

    std::vector<core::models::PantryItem> pantryItems;
    for (const auto& line : readLines(directory / "pantry.tsv")) {
        const auto fields = json::decodeRow(line);
        if (fields.size() < 7) continue;
        pantryItems.push_back(core::models::PantryItem {
            .id = fields[0],
            .userId = fields[1],
            .name = fields[2],
            .unit = fields[3],
            .quantity = toDouble(fields[4]),
            .lowStockThreshold = toDouble(fields[5]),
            .updatedAt = core::models::Date::fromString(fields[6]),
        });
    }

    std::vector<core::models::ShoppingItem> shoppingItems;
    for (const auto& line : readLines(directory / "shopping.tsv")) {
        const auto fields = json::decodeRow(line);
        if (fields.size() < 8) continue;
        shoppingItems.push_back(core::models::ShoppingItem {
            .id = fields[0],
            .userId = fields[1],
            .name = fields[2],
            .category = fields[3],
            .plannedQuantity = toDouble(fields[4]),
            .unit = fields[5],
            .purchased = toBool(fields[6]),
            .createdAt = core::models::Date::fromString(fields[7]),
        });
    }
    backend.shopping().loadState(std::move(pantryItems), std::move(shoppingItems));
}

}  // namespace finsight::data::storage
