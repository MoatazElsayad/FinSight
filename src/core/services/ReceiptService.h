#pragma once

#include "../models/Receipt.h"

#include <cstddef>
#include <string>
#include <vector>

namespace finsight::core::services {

class TransactionService;

class ReceiptService {
public:
    models::ReceiptDocument uploadReceipt(const std::string& userId,
                                          const std::string& fileName,
                                          const std::string& rawText,
                                          const models::Date& uploadedAt);
    std::vector<models::ReceiptDocument> listReceipts(const std::string& userId) const;
    models::ReceiptParseResult parseReceipt(const std::string& userId,
                                            const std::string& receiptId,
                                            const TransactionService& transactionService);
    models::Transaction confirmReceiptAsTransaction(const std::string& userId,
                                                    const models::ReceiptConfirmation& confirmation,
                                                    TransactionService& transactionService);
    std::vector<models::ReceiptDocument> allReceipts() const;
    std::vector<models::ReceiptParseResult> allParsedReceipts() const;
    void loadState(std::vector<models::ReceiptDocument> receipts,
                   std::vector<models::ReceiptParseResult> parsedReceipts);

private:
    std::string nextReceiptId();
    static std::vector<std::string> splitLines(const std::string& text);
    static std::optional<double> firstAmount(const std::vector<std::string>& lines);
    static std::optional<models::Date> firstDate(const std::vector<std::string>& lines);
    static std::string firstMerchant(const std::vector<std::string>& lines);

    std::vector<models::ReceiptDocument> receipts_;
    std::vector<models::ReceiptParseResult> parsedReceipts_;
    std::size_t nextReceiptId_ {1};
};

}  // namespace finsight::core::services
