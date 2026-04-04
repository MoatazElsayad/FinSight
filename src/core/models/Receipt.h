#pragma once

#include "Common.h"
#include "Transaction.h"

#include <optional>
#include <string>
#include <vector>

using namespace std;

namespace finsight::core::models {

// Tracks the current processing stage of a receipt.
enum class ReceiptStatus {
    Uploaded,
    Parsed,
    Confirmed
};

// Stores a raw uploaded receipt document.
struct ReceiptDocument {
    std::string id;
    std::string userId;
    std::string fileName;
    std::string rawText;
    ReceiptStatus status {ReceiptStatus::Uploaded};
    Date uploadedAt;
};

// Stores parsed information extracted from a receipt.
struct ReceiptParseResult {
    std::string receiptId;
    std::string merchant;
    std::optional<Date> transactionDate;
    std::optional<double> amount;
    std::string suggestedCategoryId;
    std::string confidenceNotes;
    std::vector<std::string> extractedLines;
};

// Stores the final user-confirmed transaction values from a receipt.
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
