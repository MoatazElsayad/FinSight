#include "ReceiptService.h"

#include "TransactionService.h"

#include <algorithm>
#include <cctype>
#include <regex>
#include <sstream>
#include <stdexcept>

using namespace std;

namespace finsight::core::services {

namespace {

std::string trim(std::string value) {
    value.erase(value.begin(), std::find_if(value.begin(), value.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    }));
    value.erase(std::find_if(value.rbegin(), value.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
    }).base(), value.end());
    return value;
}

std::optional<double> amountFromLine(const std::string& line) {
    const std::regex amountPattern(R"(([0-9]+(?:[.,][0-9]{1,2})?))");
    const std::regex datePattern(R"(\d{4}-\d{2}-\d{2})");
    const bool lineContainsDate = std::regex_search(line, datePattern);
    std::optional<double> lastAmount;

    for (std::sregex_iterator it(line.begin(), line.end(), amountPattern), end; it != end; ++it) {
        std::string token = (*it)[1].str();
        std::replace(token.begin(), token.end(), ',', '.');
        try {
            const double value = std::stod(token);
            const bool looksLikeDateYear = lineContainsDate && value >= 1900.0 && value <= 2100.0;
            if (value > 0.0 && !looksLikeDateYear) {
                lastAmount = value;
            }
        } catch (...) {
        }
    }

    return lastAmount;
}

}  // namespace

// Stores a newly uploaded receipt document.
models::ReceiptDocument ReceiptService::uploadReceipt(const std::string& userId,
                                                      const std::string& fileName,
                                                      const std::string& rawText,
                                                      const models::Date& uploadedAt) {
    if (userId.empty() || fileName.empty()) {
        throw std::invalid_argument("Receipt requires user and file name.");
    }
    if (trim(rawText).empty()) {
        throw std::invalid_argument("Receipt text cannot be empty.");
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

// Returns one receipt when it belongs to the user.
std::optional<models::ReceiptDocument> ReceiptService::findReceipt(const std::string& userId,
                                                                   const std::string& receiptId) const {
    for (const auto& receipt : receipts_) {
        if (receipt.id == receiptId && receipt.userId == userId) {
            return receipt;
        }
    }
    return std::nullopt;
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
        .confidenceNotes = "Heuristic parsing from receipt text. Review before saving.",
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

// Returns the latest parsed result for a receipt.
std::optional<models::ReceiptParseResult> ReceiptService::findParsedReceipt(const std::string& receiptId) const {
    for (const auto& parsed : parsedReceipts_) {
        if (parsed.receiptId == receiptId) {
            return parsed;
        }
    }
    return std::nullopt;
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
    if (confirmation.title.empty()) {
        throw std::invalid_argument("Receipt confirmation requires a transaction title.");
    }
    if (confirmation.categoryId.empty()) {
        throw std::invalid_argument("Receipt confirmation requires a category.");
    }
    if (confirmation.amount <= 0.0) {
        throw std::invalid_argument("Receipt confirmation amount must be greater than 0.");
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
        .tags = {"receipt", confirmation.receiptId},
    };

    auto savedTransaction = transactionService.addTransaction(transaction);
    receiptIt->status = models::ReceiptStatus::Confirmed;
    return savedTransaction;
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
        line = trim(line);
        if (!line.empty()) {
            lines.push_back(line);
        }
    }
    return lines;
}

// Extracts the first amount-like value from receipt lines.
std::optional<double> ReceiptService::firstAmount(const std::vector<std::string>& lines) {
    for (const auto& line : lines) {
        if (models::containsCaseInsensitive(line, "grand total") ||
            models::containsCaseInsensitive(line, "total") ||
            models::containsCaseInsensitive(line, "amount due") ||
            models::containsCaseInsensitive(line, "paid")) {
            if (auto amount = amountFromLine(line)) {
                return amount;
            }
        }
    }

    std::optional<double> bestAmount;
    for (const auto& line : lines) {
        if (auto amount = amountFromLine(line)) {
            if (!bestAmount || *amount > *bestAmount) {
                bestAmount = amount;
            }
        }
    }
    return bestAmount;
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
