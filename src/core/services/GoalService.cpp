#include "GoalService.h"

#include <algorithm>
#include <regex>
#include <sstream>
#include <stdexcept>

using namespace std;

namespace finsight::core::services {

// Creates a new goal and marks it complete if already reached.
models::Goal GoalService::createGoal(models::Goal goal) {
    if (goal.userId.empty() || goal.title.empty() || goal.targetAmount <= 0.0) {
        throw std::invalid_argument("Goal requires user, title, and positive target.");
    }
    goal.id = nextId();
    goal.completed = goal.currentAmount >= goal.targetAmount;
    goals_.push_back(goal);
    return goal;
}

// Updates the current saved amount for a goal.
models::Goal GoalService::updateProgress(const std::string& userId,
                                         const std::string& goalId,
                                         double currentAmount) {
    for (auto& goal : goals_) {
        if (goal.id == goalId && goal.userId == userId) {
            goal.currentAmount = currentAmount;
            goal.completed = goal.currentAmount >= goal.targetAmount;
            return goal;
        }
    }
    throw std::out_of_range("Goal not found.");
}

// Deletes one stored goal.
void GoalService::deleteGoal(const std::string& userId, const std::string& goalId) {
    auto iterator = std::find_if(goals_.begin(), goals_.end(), [&](const auto& goal) {
        return goal.id == goalId && goal.userId == userId;
    });
    if (iterator == goals_.end()) {
        throw std::out_of_range("Goal not found.");
    }
    goals_.erase(iterator);
}

// Returns all goals for one user.
std::vector<models::Goal> GoalService::listGoals(const std::string& userId) const {
    std::vector<models::Goal> result;
    for (const auto& goal : goals_) {
        if (goal.userId == userId) {
            result.push_back(goal);
        }
    }
    return result;
}

// Returns every stored goal.
std::vector<models::Goal> GoalService::allGoals() const {
    return goals_;
}

// Restores goals from persisted state and resets id generation.
void GoalService::loadGoals(std::vector<models::Goal> goals) {
    goals_ = std::move(goals);
    std::size_t maxId = 0;
    const std::regex pattern(R"(gol-(\d+))");
    for (const auto& goal : goals_) {
        std::smatch match;
        if (std::regex_match(goal.id, match, pattern)) {
            maxId = std::max<std::size_t>(maxId, static_cast<std::size_t>(std::stoull(match[1].str())));
        }
    }
    nextGoalId_ = maxId + 1;
}

// Builds the next goal id string.
std::string GoalService::nextId() {
    std::ostringstream stream;
    stream << "gol-" << nextGoalId_++;
    return stream.str();
}

}  // namespace finsight::core::services
