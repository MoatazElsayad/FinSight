#pragma once

#include "../models/Goal.h"

#include <cstddef>
#include <string>
#include <vector>

using namespace std;

namespace finsight::core::services {

class GoalService {
public:
    // Creates a new financial goal.
    models::Goal createGoal(models::Goal goal);
    // Updates the saved progress for a goal.
    models::Goal updateProgress(const std::string& userId,
                                const std::string& goalId,
                                double currentAmount);
    // Deletes a goal.
    void deleteGoal(const std::string& userId, const std::string& goalId);
    // Returns all goals for one user.
    std::vector<models::Goal> listGoals(const std::string& userId) const;
    // Returns every stored goal.
    std::vector<models::Goal> allGoals() const;
    // Replaces the stored goals from persisted data.
    void loadGoals(std::vector<models::Goal> goals);

private:
    // Generates the next goal id.
    std::string nextId();

    std::vector<models::Goal> goals_;
    std::size_t nextGoalId_ {1};
};

}  // namespace finsight::core::services
