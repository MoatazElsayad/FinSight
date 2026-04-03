#pragma once

#include "Common.h"
#include "Transaction.h"

#include <optional>
#include <string>
#include <vector>

namespace finsight::core::models {

enum class ReceiptStatus {
    Uploaded,
    Parsed,
    Confirmed
};

struct ReceiptDocument {
    std::string id;
    std::string userId;
    std::string fileName;
    std::string rawText;
    ReceiptStatus status {ReceiptStatus::Uploaded};
    Date uploadedAt;
};

struct ReceiptParseResult {
    std::string receiptId;
    std::string merchant;
    std::optional<Date> transactionDate;
    std::optional<double> amount;
    std::string suggestedCategoryId;
    std::string confidenceNotes;
    std::vector<std::string> extractedLines;
};

struct ReceiptConfirmation {
    std::string receiptId;
    std::string title;
    std::string description;
    std::string merchant;
    std::string categoryId;
    TransactionType type {TransactionType::Expense};
    double amount {0.0};
    Date date;
};

}  // namespace finsight::core::models
