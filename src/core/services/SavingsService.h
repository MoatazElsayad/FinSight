#pragma once

#include "../models/Investment.h"
#include "../models/Savings.h"

#include <cstddef>
#include <optional>
#include <string>
#include <vector>

using namespace std;

namespace finsight::core::services {

class SavingsService {
public:
    // Adds a savings deposit or withdrawal entry.
    models::SavingsEntry addEntry(models::SavingsEntry entry);
    // Convenience helper for recording money added to savings.
    models::SavingsEntry addDeposit(const std::string& userId,
                                    double amount,
                                    const models::Date& date,
                                    const std::string& note = {});
    // Convenience helper for recording money removed from savings.
    models::SavingsEntry addWithdrawal(const std::string& userId,
                                       double amount,
                                       const models::Date& date,
                                       const std::string& note = {});
    // Deletes one savings entry.
    void deleteEntry(const std::string& userId, const std::string& entryId);
    // Returns all savings entries for one user.
    std::vector<models::SavingsEntry> listEntries(const std::string& userId) const;

    // Creates or updates the user's savings goals.
    models::SavingsGoal setGoal(const std::string& userId,
                                double monthlyTarget,
                                double longTermTarget,
                                const models::Date& targetDate);
    // Returns the user's savings target if configured.
    std::optional<models::SavingsGoal> getGoal(const std::string& userId) const;
    // Builds a savings summary for a given month.
    models::SavingsOverview summarize(const std::string& userId,
                                      const models::YearMonth& period) const;

    // Adds a new investment position.
    models::Investment addInvestment(models::Investment investment);
    // Updates the current market rate for one investment.
    models::Investment updateInvestmentRate(const std::string& userId,
                                            const std::string& investmentId,
                                            double currentRate);
    // Updates the editable fields for one investment.
    models::Investment updateInvestment(const std::string& userId,
                                        const std::string& investmentId,
                                        const models::Investment& updatedInvestment);
    // Deletes one investment.
    void deleteInvestment(const std::string& userId, const std::string& investmentId);
    // Returns all investments for one user.
    std::vector<models::Investment> listInvestments(const std::string& userId) const;
    // Calculates current value snapshots for a user's investments.
    std::vector<models::InvestmentSnapshot> investmentSnapshots(const std::string& userId) const;
    // Returns every stored savings entry.
    std::vector<models::SavingsEntry> allEntries() const;
    // Returns every stored investment.
    std::vector<models::Investment> allInvestments() const;
    // Returns every stored savings goal.
    std::vector<models::SavingsGoal> allGoals() const;
    // Replaces savings state from persisted data.
    void loadState(std::vector<models::SavingsEntry> entries,
                   std::vector<models::Investment> investments,
                   std::vector<models::SavingsGoal> goals);

private:
    // Generates the next savings entry id.
    std::string nextEntryId();
    // Generates the next investment id.
    std::string nextInvestmentId();
    // Generates the next savings-goal id.
    std::string nextGoalId();

    std::vector<models::SavingsEntry> entries_;
    std::vector<models::Investment> investments_;
    std::vector<models::SavingsGoal> goals_;
    std::size_t nextEntryId_ {1};
    std::size_t nextInvestmentId_ {1};
    std::size_t nextGoalId_ {1};
};

}  // namespace finsight::core::services
