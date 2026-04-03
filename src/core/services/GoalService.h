#pragma once

#include "../models/Goal.h"

#include <cstddef>
#include <string>
#include <vector>

namespace finsight::core::services {

class GoalService {
public:
    models::Goal createGoal(models::Goal goal);
    models::Goal updateProgress(const std::string& userId,
                                const std::string& goalId,
                                double currentAmount);
    void deleteGoal(const std::string& userId, const std::string& goalId);
    std::vector<models::Goal> listGoals(const std::string& userId) const;
    std::vector<models::Goal> allGoals() const;
    void loadGoals(std::vector<models::Goal> goals);

private:
    std::string nextId();

    std::vector<models::Goal> goals_;
    std::size_t nextGoalId_ {1};
};

}  // namespace finsight::core::services
