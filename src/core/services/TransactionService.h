#pragma once

#include "../models/Category.h"
#include "../models/Transaction.h"

#include <cstddef>
#include <string>
#include <vector>

using namespace std;

namespace finsight::core::services {

class TransactionService {
public:
    // Builds the service with default built-in categories.
    TransactionService();

    // Returns all stored categories.
    const std::vector<models::Category>& getCategories() const;
    // Returns built-in plus user-owned categories.
    std::vector<models::Category> getCategoriesForUser(const std::string& userId) const;
    // Creates a new category.
    models::Category createCategory(const std::string& userId,
                                    const std::string& name,
                                    models::CategoryKind kind,
                                    const std::string& icon,
                                    bool builtIn = false);
    // Deletes one custom category.
    void deleteCategory(const std::string& userId, const std::string& categoryId);

    // Adds a new transaction.
    models::Transaction addTransaction(models::Transaction transaction);
    // Updates an existing transaction.
    models::Transaction updateTransaction(const std::string& userId,
                                          const std::string& transactionId,
                                          const models::Transaction& updated);
    // Deletes one transaction.
    void deleteTransaction(const std::string& userId, const std::string& transactionId);
    // Deletes many transactions in one call.
    void bulkDeleteTransactions(const std::string& userId, const std::vector<std::string>& transactionIds);

    // Returns all transactions owned by a user.
    std::vector<models::Transaction> listTransactions(const std::string& userId) const;
    // Returns transactions matching the requested filters.
    std::vector<models::Transaction> filterTransactions(const std::string& userId,
                                                        const models::TransactionFilter& filter) const;
    // Sums one transaction type for a selected month.
    double sumTransactions(const std::string& userId,
                           models::TransactionType type,
                           const models::YearMonth& period) const;
    // Calculates category spending for one month.
    double spentForCategory(const std::string& userId,
                            const std::string& categoryId,
                            const models::YearMonth& period) const;
    // Returns one category by id or throws if it does not exist.
    const models::Category& requireCategory(const std::string& categoryId) const;
    // Returns every stored transaction.
    std::vector<models::Transaction> allTransactions() const;
    // Replaces category and transaction state from persisted data.
    void loadState(std::vector<models::Category> categories,
                   std::vector<models::Transaction> transactions);

private:
    // Generates the next category or transaction id.
    std::string nextId(const std::string& prefix);
    // Validates transaction contents before saving.
    void ensureTransactionIsValid(const models::Transaction& transaction) const;

    std::vector<models::Category> categories_;
    std::vector<models::Transaction> transactions_;
    std::size_t nextCategoryId_ {1};
    std::size_t nextTransactionId_ {1};
};

}  // namespace finsight::core::services
