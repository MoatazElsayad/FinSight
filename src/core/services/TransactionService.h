#pragma once

#include "../models/Category.h"
#include "../models/Transaction.h"

#include <cstddef>
#include <string>
#include <vector>

namespace finsight::core::services {

class TransactionService {
public:
    TransactionService();

    const std::vector<models::Category>& getCategories() const;
    std::vector<models::Category> getCategoriesForUser(const std::string& userId) const;
    models::Category createCategory(const std::string& userId,
                                    const std::string& name,
                                    models::CategoryKind kind,
                                    const std::string& icon,
                                    bool builtIn = false);
    void deleteCategory(const std::string& userId, const std::string& categoryId);

    models::Transaction addTransaction(models::Transaction transaction);
    models::Transaction updateTransaction(const std::string& userId,
                                          const std::string& transactionId,
                                          const models::Transaction& updated);
    void deleteTransaction(const std::string& userId, const std::string& transactionId);
    void bulkDeleteTransactions(const std::string& userId, const std::vector<std::string>& transactionIds);

    std::vector<models::Transaction> listTransactions(const std::string& userId) const;
    std::vector<models::Transaction> filterTransactions(const std::string& userId,
                                                        const models::TransactionFilter& filter) const;
    double sumTransactions(const std::string& userId,
                           models::TransactionType type,
                           const models::YearMonth& period) const;
    double spentForCategory(const std::string& userId,
                            const std::string& categoryId,
                            const models::YearMonth& period) const;
    const models::Category& requireCategory(const std::string& categoryId) const;
    std::vector<models::Transaction> allTransactions() const;
    void loadState(std::vector<models::Category> categories,
                   std::vector<models::Transaction> transactions);

private:
    std::string nextId(const std::string& prefix);
    void ensureTransactionIsValid(const models::Transaction& transaction) const;

    std::vector<models::Category> categories_;
    std::vector<models::Transaction> transactions_;
    std::size_t nextCategoryId_ {1};
    std::size_t nextTransactionId_ {1};
};

}  // namespace finsight::core::services
