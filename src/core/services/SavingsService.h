#pragma once

#include "../models/Investment.h"
#include "../models/Savings.h"

#include <cstddef>
#include <string>
#include <vector>

namespace finsight::core::services {

class SavingsService {
public:
    models::SavingsEntry addEntry(models::SavingsEntry entry);
    void deleteEntry(const std::string& userId, const std::string& entryId);
    std::vector<models::SavingsEntry> listEntries(const std::string& userId) const;

    models::SavingsGoal setGoal(const std::string& userId,
                                double monthlyTarget,
                                double longTermTarget,
                                const models::Date& targetDate);
    models::SavingsOverview summarize(const std::string& userId,
                                      const models::YearMonth& period) const;

    models::Investment addInvestment(models::Investment investment);
    models::Investment updateInvestmentRate(const std::string& userId,
                                            const std::string& investmentId,
                                            double currentRate);
    void deleteInvestment(const std::string& userId, const std::string& investmentId);
    std::vector<models::Investment> listInvestments(const std::string& userId) const;
    std::vector<models::InvestmentSnapshot> investmentSnapshots(const std::string& userId) const;
    std::vector<models::SavingsEntry> allEntries() const;
    std::vector<models::Investment> allInvestments() const;
    std::vector<models::SavingsGoal> allGoals() const;
    void loadState(std::vector<models::SavingsEntry> entries,
                   std::vector<models::Investment> investments,
                   std::vector<models::SavingsGoal> goals);

private:
    std::string nextEntryId();
    std::string nextInvestmentId();
    std::string nextGoalId();

    std::vector<models::SavingsEntry> entries_;
    std::vector<models::Investment> investments_;
    std::vector<models::SavingsGoal> goals_;
    std::size_t nextEntryId_ {1};
    std::size_t nextInvestmentId_ {1};
    std::size_t nextGoalId_ {1};
};

}  // namespace finsight::core::services
