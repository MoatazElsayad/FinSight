#pragma once

#include "../models/Receipt.h"

#include <cstddef>
#include <string>
#include <vector>

using namespace std;

namespace finsight::core::services {

class TransactionService;

class ReceiptService {
public:
    // Stores a newly uploaded receipt document.
    models::ReceiptDocument uploadReceipt(const std::string& userId,
                                          const std::string& fileName,
                                          const std::string& rawText,
                                          const models::Date& uploadedAt);
    // Returns all receipts owned by a user.
    std::vector<models::ReceiptDocument> listReceipts(const std::string& userId) const;
    // Returns one receipt if it belongs to the user.
    std::optional<models::ReceiptDocument> findReceipt(const std::string& userId,
                                                       const std::string& receiptId) const;
    // Parses raw receipt text into structured suggestions.
    models::ReceiptParseResult parseReceipt(const std::string& userId,
                                            const std::string& receiptId,
                                            const TransactionService& transactionService);
    // Returns the latest parsed result for a receipt if present.
    std::optional<models::ReceiptParseResult> findParsedReceipt(const std::string& receiptId) const;
    // Converts a confirmed receipt into a stored transaction.
    models::Transaction confirmReceiptAsTransaction(const std::string& userId,
                                                    const models::ReceiptConfirmation& confirmation,
                                                    TransactionService& transactionService);
    // Returns every stored receipt.
    std::vector<models::ReceiptDocument> allReceipts() const;
    // Returns every stored parsed receipt result.
    std::vector<models::ReceiptParseResult> allParsedReceipts() const;
    // Replaces receipt state from persisted data.
    void loadState(std::vector<models::ReceiptDocument> receipts,
                   std::vector<models::ReceiptParseResult> parsedReceipts);

private:
    // Generates the next receipt id.
    std::string nextReceiptId();
    // Splits raw OCR text into non-empty lines.
    static std::vector<std::string> splitLines(const std::string& text);
    // Finds the first amount-like value inside receipt text.
    static std::optional<double> firstAmount(const std::vector<std::string>& lines);
    // Finds the first date-like value inside receipt text.
    static std::optional<models::Date> firstDate(const std::vector<std::string>& lines);
    // Uses the first line as a merchant hint.
    static std::string firstMerchant(const std::vector<std::string>& lines);

    std::vector<models::ReceiptDocument> receipts_;
    std::vector<models::ReceiptParseResult> parsedReceipts_;
    std::size_t nextReceiptId_ {1};
};

}  // namespace finsight::core::services
