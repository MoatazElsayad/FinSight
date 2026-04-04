#include "ReceiptService.h"

#include "TransactionService.h"

#include <algorithm>
#include <regex>
#include <sstream>
#include <stdexcept>

using namespace std;

namespace finsight::core::services {

// Stores a newly uploaded receipt document.
models::ReceiptDocument ReceiptService::uploadReceipt(const std::string& userId,
                                                      const std::string& fileName,
                                                      const std::string& rawText,
                                                      const models::Date& uploadedAt) {
    if (userId.empty() || fileName.empty()) {
        throw std::invalid_argument("Receipt requires user and file name.");
    }

    models::ReceiptDocument receipt {
        .id = nextReceiptId(),
        .userId = userId,
        .fileName = fileName,
        .rawText = rawText,
        .status = models::ReceiptStatus::Uploaded,
        .uploadedAt = uploadedAt,
    };
    receipts_.push_back(receipt);
    return receipt;
}

// Returns all receipts owned by one user.
std::vector<models::ReceiptDocument> ReceiptService::listReceipts(const std::string& userId) const {
    std::vector<models::ReceiptDocument> result;
    for (const auto& receipt : receipts_) {
        if (receipt.userId == userId) {
            result.push_back(receipt);
        }
    }
    return result;
}

// Parses receipt text into a lightweight structured result.
models::ReceiptParseResult ReceiptService::parseReceipt(const std::string& userId,
                                                        const std::string& receiptId,
                                                        const TransactionService& transactionService) {
    auto receiptIt = std::find_if(receipts_.begin(), receipts_.end(), [&](const auto& receipt) {
        return receipt.id == receiptId && receipt.userId == userId;
    });
    if (receiptIt == receipts_.end()) {
        throw std::out_of_range("Receipt not found.");
    }

    const auto lines = splitLines(receiptIt->rawText);
    models::ReceiptParseResult result {
        .receiptId = receiptId,
        .merchant = firstMerchant(lines),
        .transactionDate = firstDate(lines),
        .amount = firstAmount(lines),
        .confidenceNotes = "Heuristic parsing from raw OCR text. Review before saving.",
        .extractedLines = lines,
    };

    const auto categories = transactionService.getCategoriesForUser(userId);
    for (const auto& category : categories) {
        for (const auto& line : lines) {
            if (models::containsCaseInsensitive(line, category.name)) {
                result.suggestedCategoryId = category.id;
                break;
            }
        }
        if (!result.suggestedCategoryId.empty()) {
            break;
        }
    }

    receiptIt->status = models::ReceiptStatus::Parsed;

    parsedReceipts_.erase(std::remove_if(parsedReceipts_.begin(),
                                         parsedReceipts_.end(),
                                         [&](const auto& parsed) {
                                             return parsed.receiptId == receiptId;
                                         }),
                          parsedReceipts_.end());
    parsedReceipts_.push_back(result);
    return result;
}

// Converts a confirmed receipt into a saved transaction.
models::Transaction ReceiptService::confirmReceiptAsTransaction(
    const std::string& userId,
    const models::ReceiptConfirmation& confirmation,
    TransactionService& transactionService) {
    auto receiptIt = std::find_if(receipts_.begin(), receipts_.end(), [&](const auto& receipt) {
        return receipt.id == confirmation.receiptId && receipt.userId == userId;
    });
    if (receiptIt == receipts_.end()) {
        throw std::out_of_range("Receipt not found.");
    }

    models::Transaction transaction {
        .userId = userId,
        .title = confirmation.title,
        .description = confirmation.description,
        .categoryId = confirmation.categoryId,
        .type = confirmation.type,
        .amount = confirmation.amount,
        .date = confirmation.date,
        .merchant = confirmation.merchant,
    };

    receiptIt->status = models::ReceiptStatus::Confirmed;
    return transactionService.addTransaction(transaction);
}

// Returns every stored receipt document.
std::vector<models::ReceiptDocument> ReceiptService::allReceipts() const {
    return receipts_;
}

// Returns every stored parsed receipt result.
std::vector<models::ReceiptParseResult> ReceiptService::allParsedReceipts() const {
    return parsedReceipts_;
}

// Restores receipt state from persisted data.
void ReceiptService::loadState(std::vector<models::ReceiptDocument> receipts,
                               std::vector<models::ReceiptParseResult> parsedReceipts) {
    receipts_ = std::move(receipts);
    parsedReceipts_ = std::move(parsedReceipts);

    std::size_t maxId = 0;
    const std::regex pattern(R"(receipt-(\d+))");
    for (const auto& receipt : receipts_) {
        std::smatch match;
        if (std::regex_match(receipt.id, match, pattern)) {
            maxId = std::max<std::size_t>(maxId, static_cast<std::size_t>(std::stoull(match[1].str())));
        }
    }
    nextReceiptId_ = maxId + 1;
}

// Builds the next receipt id string.
std::string ReceiptService::nextReceiptId() {
    std::ostringstream stream;
    stream << "receipt-" << nextReceiptId_++;
    return stream.str();
}

// Splits raw receipt text into non-empty lines.
std::vector<std::string> ReceiptService::splitLines(const std::string& text) {
    std::vector<std::string> lines;
    std::istringstream stream(text);
    std::string line;
    while (std::getline(stream, line)) {
        if (!line.empty()) {
            lines.push_back(line);
        }
    }
    return lines;
}

// Extracts the first amount-like value from receipt lines.
std::optional<double> ReceiptService::firstAmount(const std::vector<std::string>& lines) {
    const std::regex amountPattern(R"((\d+(?:\.\d{1,2})?))");
    for (const auto& line : lines) {
        std::smatch match;
        if (std::regex_search(line, match, amountPattern)) {
            try {
                return std::stod(match[1].str());
            } catch (...) {
            }
        }
    }
    return std::nullopt;
}

// Extracts the first date-like value from receipt lines.
std::optional<models::Date> ReceiptService::firstDate(const std::vector<std::string>& lines) {
    const std::regex datePattern(R"((\d{4}-\d{2}-\d{2}))");
    for (const auto& line : lines) {
        std::smatch match;
        if (std::regex_search(line, match, datePattern)) {
            try {
                return models::Date::fromString(match[1].str());
            } catch (...) {
            }
        }
    }
    return std::nullopt;
}

// Uses the first line as a simple merchant guess.
std::string ReceiptService::firstMerchant(const std::vector<std::string>& lines) {
    return lines.empty() ? std::string {} : lines.front();
}

}  // namespace finsight::core::services
