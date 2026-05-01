#pragma once

#include "../models/Email.h"
#include "../models/Transaction.h"

#include <vector>

using namespace std;

namespace finsight::core::services {

class AIService;
class AuthService;
class BudgetService;
class EmailService;
class TransactionService;

class BudgetAlertService {
public:
    // Sends budget alert emails when a new expense transaction pushes spending over a budget limit.
    vector<models::EmailSendResult> notifyBudgetExceededByTransaction(const models::Transaction& transaction,
                                                                      const AIService& aiService,
                                                                      const AuthService& authService,
                                                                      const TransactionService& transactionService,
                                                                      const BudgetService& budgetService,
                                                                      const EmailService& emailService) const;

private:
    // Formats a money value in a compact user-facing style.
    static string formatMoney(double value);
};

}  // namespace finsight::core::services
