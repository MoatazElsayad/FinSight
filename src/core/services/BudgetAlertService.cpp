#include "BudgetAlertService.h"

#include "AuthService.h"
#include "BudgetService.h"
#include "EmailService.h"
#include "TransactionService.h"

#include <iomanip>
#include <sstream>

using namespace std;

namespace finsight::core::services {

// Sends one alert email if the supplied expense transaction newly crosses a budget limit.
vector<models::EmailSendResult> BudgetAlertService::notifyBudgetExceededByTransaction(
    const models::Transaction& transaction,
    const AuthService& authService,
    const TransactionService& transactionService,
    const BudgetService& budgetService,
    const EmailService& emailService) const {
    vector<models::EmailSendResult> results;

    if (transaction.type != models::TransactionType::Expense) {
        return results;
    }

    const models::YearMonth period {transaction.date.year, transaction.date.month};
    const auto statuses = budgetService.summarizeBudgets(transaction.userId, period, transactionService);

    for (const auto& status : statuses) {
        if (status.budget.categoryId != transaction.categoryId || !status.overspent) {
            continue;
        }

        const double previousSpent = status.spent - transaction.amount;
        if (previousSpent > status.budget.limit) {
            continue;
        }

        const auto& user = authService.getUser(transaction.userId);
        const auto& category = transactionService.requireCategory(transaction.categoryId);

        ostringstream subject;
        subject << "FinSight budget alert: " << category.name << " budget exceeded";

        ostringstream body;
        body << "Hello " << user.fullName << ",\n\n";
        body << "Your " << category.name << " budget for "
             << status.budget.period.year << '-'
             << setw(2) << setfill('0') << status.budget.period.month
             << " has been exceeded.\n\n";
        body << "Budget limit: " << formatMoney(status.budget.limit) << "\n";
        body << "Current spending: " << formatMoney(status.spent) << "\n";
        body << "Amount over budget: " << formatMoney(status.spent - status.budget.limit) << "\n";
        body << "Latest transaction: " << transaction.title << " (" << formatMoney(transaction.amount) << ")\n";
        body << "Transaction date: " << transaction.date.toString() << "\n\n";
        body << "Open FinSight to review your spending and adjust your budget if needed.\n";

        results.push_back(emailService.sendEmail(models::EmailMessage {
            .to = user.email,
            .subject = subject.str(),
            .body = body.str(),
        }));
    }

    return results;
}

// Formats money values with two decimal places for email messages.
string BudgetAlertService::formatMoney(double value) {
    ostringstream stream;
    stream << fixed << setprecision(2) << value;
    return stream.str();
}

}  // namespace finsight::core::services
