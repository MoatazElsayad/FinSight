#include "SavingsService.h"

#include <algorithm>
#include <regex>
#include <sstream>
#include <stdexcept>

using namespace std;

namespace finsight::core::services {

namespace {

void validateSavingsEntry(const models::SavingsEntry& entry) {
    if (entry.userId.empty()) {
        throw std::invalid_argument("Savings entry requires a user.");
    }
    if (entry.amount <= 0.0) {
        throw std::invalid_argument("Savings entry amount must be greater than 0.");
    }
}

void validateSavingsGoal(const std::string& userId,
                         double monthlyTarget,
                         double longTermTarget) {
    if (userId.empty()) {
        throw std::invalid_argument("Savings goal requires a user.");
    }
    if (monthlyTarget < 0.0 || longTermTarget < 0.0) {
        throw std::invalid_argument("Savings targets cannot be negative.");
    }
}

void validateInvestment(const models::Investment& investment) {
    if (investment.userId.empty()) {
        throw std::invalid_argument("Investment requires a user.");
    }
    if (investment.assetName.empty()) {
        throw std::invalid_argument("Investment requires an asset name.");
    }
    if (investment.quantity <= 0.0) {
        throw std::invalid_argument("Investment quantity must be greater than 0.");
    }
    if (investment.buyRate < 0.0 || investment.currentRate < 0.0) {
        throw std::invalid_argument("Investment rates cannot be negative.");
    }
}

}  // namespace

// Adds one savings entry to the ledger.
models::SavingsEntry SavingsService::addEntry(models::SavingsEntry entry) {
    validateSavingsEntry(entry);
    entry.id = nextEntryId();
    entries_.push_back(entry);
    return entry;
}

// Records a savings deposit.
models::SavingsEntry SavingsService::addDeposit(const std::string& userId,
                                                double amount,
                                                const models::Date& date,
                                                const std::string& note) {
    return addEntry(models::SavingsEntry {
        .userId = userId,
        .type = models::SavingsEntryType::Deposit,
        .amount = amount,
        .date = date,
        .note = note,
    });
}

// Records a savings withdrawal.
models::SavingsEntry SavingsService::addWithdrawal(const std::string& userId,
                                                   double amount,
                                                   const models::Date& date,
                                                   const std::string& note) {
    return addEntry(models::SavingsEntry {
        .userId = userId,
        .type = models::SavingsEntryType::Withdrawal,
        .amount = amount,
        .date = date,
        .note = note,
    });
}

// Deletes one savings entry.
void SavingsService::deleteEntry(const std::string& userId, const std::string& entryId) {
    auto iterator = std::find_if(entries_.begin(), entries_.end(), [&](const auto& entry) {
        return entry.id == entryId && entry.userId == userId;
    });
    if (iterator == entries_.end()) {
        throw std::out_of_range("Savings entry not found.");
    }
    entries_.erase(iterator);
}

// Returns all savings entries for one user in descending date order.
std::vector<models::SavingsEntry> SavingsService::listEntries(const std::string& userId) const {
    std::vector<models::SavingsEntry> result;
    for (const auto& entry : entries_) {
        if (entry.userId == userId) {
            result.push_back(entry);
        }
    }
    std::sort(result.begin(), result.end(), [](const auto& left, const auto& right) {
        return left.date > right.date;
    });
    return result;
}

// Creates or updates the user's savings goals.
models::SavingsGoal SavingsService::setGoal(const std::string& userId,
                                            double monthlyTarget,
                                            double longTermTarget,
                                            const models::Date& targetDate) {
    validateSavingsGoal(userId, monthlyTarget, longTermTarget);
    for (auto& goal : goals_) {
        if (goal.userId == userId) {
            goal.monthlyTarget = monthlyTarget;
            goal.longTermTarget = longTermTarget;
            goal.targetDate = targetDate;
            return goal;
        }
    }

    models::SavingsGoal goal {
        .id = nextGoalId(),
        .userId = userId,
        .monthlyTarget = monthlyTarget,
        .longTermTarget = longTermTarget,
        .targetDate = targetDate,
    };
    goals_.push_back(goal);
    return goal;
}

// Returns the user's savings target when one has been configured.
std::optional<models::SavingsGoal> SavingsService::getGoal(const std::string& userId) const {
    for (const auto& goal : goals_) {
        if (goal.userId == userId) {
            return goal;
        }
    }
    return std::nullopt;
}

// Builds a monthly savings summary.
models::SavingsOverview SavingsService::summarize(const std::string& userId,
                                                  const models::YearMonth& period) const {
    double balance = 0.0;
    double monthlySaved = 0.0;
    for (const auto& entry : entries_) {
        if (entry.userId != userId) {
            continue;
        }
        const double signedAmount = entry.type == models::SavingsEntryType::Deposit
                                        ? entry.amount
                                        : -entry.amount;
        balance += signedAmount;
        if (models::inMonth(entry.date, period)) {
            monthlySaved += signedAmount;
        }
    }

    models::SavingsOverview overview {.currentBalance = balance, .monthlySaved = monthlySaved};
    for (const auto& goal : goals_) {
        if (goal.userId == userId) {
            overview.monthlyTarget = goal.monthlyTarget;
            overview.longTermTarget = goal.longTermTarget;
            overview.progressToMonthlyTarget = goal.monthlyTarget <= 0.0
                                                   ? 0.0
                                                   : monthlySaved / goal.monthlyTarget;
            break;
        }
    }
    return overview;
}

// Adds a new investment position.
models::Investment SavingsService::addInvestment(models::Investment investment) {
    validateInvestment(investment);
    investment.id = nextInvestmentId();
    investments_.push_back(investment);
    return investment;
}

// Updates the latest rate for one investment.
models::Investment SavingsService::updateInvestmentRate(const std::string& userId,
                                                        const std::string& investmentId,
                                                        double currentRate) {
    if (currentRate < 0.0) {
        throw std::invalid_argument("Investment current rate cannot be negative.");
    }
    for (auto& investment : investments_) {
        if (investment.id == investmentId && investment.userId == userId) {
            investment.currentRate = currentRate;
            return investment;
        }
    }
    throw std::out_of_range("Investment not found.");
}

// Updates the editable fields for one investment.
models::Investment SavingsService::updateInvestment(const std::string& userId,
                                                    const std::string& investmentId,
                                                    const models::Investment& updatedInvestment) {
    models::Investment candidate = updatedInvestment;
    candidate.id = investmentId;
    candidate.userId = userId;
    validateInvestment(candidate);

    for (auto& investment : investments_) {
        if (investment.id == investmentId && investment.userId == userId) {
            investment = candidate;
            return investment;
        }
    }
    throw std::out_of_range("Investment not found.");
}

// Deletes one investment.
void SavingsService::deleteInvestment(const std::string& userId, const std::string& investmentId) {
    auto iterator = std::find_if(investments_.begin(), investments_.end(), [&](const auto& investment) {
        return investment.id == investmentId && investment.userId == userId;
    });
    if (iterator == investments_.end()) {
        throw std::out_of_range("Investment not found.");
    }
    investments_.erase(iterator);
}

// Returns all investments for one user.
std::vector<models::Investment> SavingsService::listInvestments(const std::string& userId) const {
    std::vector<models::Investment> result;
    for (const auto& investment : investments_) {
        if (investment.userId == userId) {
            result.push_back(investment);
        }
    }
    return result;
}

// Calculates current value snapshots for a user's investments.
std::vector<models::InvestmentSnapshot> SavingsService::investmentSnapshots(const std::string& userId) const {
    std::vector<models::InvestmentSnapshot> result;
    for (const auto& investment : investments_) {
        if (investment.userId != userId) {
            continue;
        }
        const double investedValue = investment.quantity * investment.buyRate;
        const double currentValue = investment.quantity * investment.currentRate;
        result.push_back(models::InvestmentSnapshot {
            .investment = investment,
            .investedValue = investedValue,
            .currentValue = currentValue,
            .profitLoss = currentValue - investedValue,
        });
    }
    return result;
}

// Returns every stored savings entry.
std::vector<models::SavingsEntry> SavingsService::allEntries() const {
    return entries_;
}

// Returns every stored investment.
std::vector<models::Investment> SavingsService::allInvestments() const {
    return investments_;
}

// Returns every stored savings goal.
std::vector<models::SavingsGoal> SavingsService::allGoals() const {
    return goals_;
}

// Restores savings, investment, and goal state from persisted data.
void SavingsService::loadState(std::vector<models::SavingsEntry> entries,
                               std::vector<models::Investment> investments,
                               std::vector<models::SavingsGoal> goals) {
    entries_ = std::move(entries);
    investments_ = std::move(investments);
    goals_ = std::move(goals);

    std::size_t maxEntryId = 0;
    std::size_t maxInvestmentId = 0;
    std::size_t maxGoalId = 0;
    const std::regex entryPattern(R"(svg-(\d+))");
    const std::regex investmentPattern(R"(inv-(\d+))");
    const std::regex goalPattern(R"(svg-goal-(\d+))");

    for (const auto& entry : entries_) {
        std::smatch match;
        if (std::regex_match(entry.id, match, entryPattern)) {
            maxEntryId = std::max<std::size_t>(maxEntryId, static_cast<std::size_t>(std::stoull(match[1].str())));
        }
    }
    for (const auto& investment : investments_) {
        std::smatch match;
        if (std::regex_match(investment.id, match, investmentPattern)) {
            maxInvestmentId = std::max<std::size_t>(maxInvestmentId, static_cast<std::size_t>(std::stoull(match[1].str())));
        }
    }
    for (const auto& goal : goals_) {
        std::smatch match;
        if (std::regex_match(goal.id, match, goalPattern)) {
            maxGoalId = std::max<std::size_t>(maxGoalId, static_cast<std::size_t>(std::stoull(match[1].str())));
        }
    }

    nextEntryId_ = maxEntryId + 1;
    nextInvestmentId_ = maxInvestmentId + 1;
    nextGoalId_ = maxGoalId + 1;
}

// Builds the next savings entry id string.
std::string SavingsService::nextEntryId() {
    std::ostringstream stream;
    stream << "svg-" << nextEntryId_++;
    return stream.str();
}

// Builds the next investment id string.
std::string SavingsService::nextInvestmentId() {
    std::ostringstream stream;
    stream << "inv-" << nextInvestmentId_++;
    return stream.str();
}

// Builds the next savings goal id string.
std::string SavingsService::nextGoalId() {
    std::ostringstream stream;
    stream << "svg-goal-" << nextGoalId_++;
    return stream.str();
}

}  // namespace finsight::core::services
