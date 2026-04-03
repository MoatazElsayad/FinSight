#include "GoalService.h"

#include <algorithm>
#include <regex>
#include <sstream>
#include <stdexcept>

namespace finsight::core::services {

models::Goal GoalService::createGoal(models::Goal goal) {
    if (goal.userId.empty() || goal.title.empty() || goal.targetAmount <= 0.0) {
        throw std::invalid_argument("Goal requires user, title, and positive target.");
    }
    goal.id = nextId();
    goal.completed = goal.currentAmount >= goal.targetAmount;
    goals_.push_back(goal);
    return goal;
}

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

void GoalService::deleteGoal(const std::string& userId, const std::string& goalId) {
    auto iterator = std::find_if(goals_.begin(), goals_.end(), [&](const auto& goal) {
        return goal.id == goalId && goal.userId == userId;
    });
    if (iterator == goals_.end()) {
        throw std::out_of_range("Goal not found.");
    }
    goals_.erase(iterator);
}

std::vector<models::Goal> GoalService::listGoals(const std::string& userId) const {
    std::vector<models::Goal> result;
    for (const auto& goal : goals_) {
        if (goal.userId == userId) {
            result.push_back(goal);
        }
    }
    return result;
}

std::vector<models::Goal> GoalService::allGoals() const {
    return goals_;
}

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

std::string GoalService::nextId() {
    std::ostringstream stream;
    stream << "gol-" << nextGoalId_++;
    return stream.str();
}

}  // namespace finsight::core::services
