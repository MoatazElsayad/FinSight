#include "GoalService.h"

#include <algorithm>
#include <regex>
#include <sstream>
#include <stdexcept>

using namespace std;

namespace finsight::core::services {

namespace {

void validateGoal(const models::Goal& goal) {
    if (goal.userId.empty()) {
        throw std::invalid_argument("Goal requires a user.");
    }
    if (goal.title.empty()) {
        throw std::invalid_argument("Goal title is required.");
    }
    if (goal.targetAmount <= 0.0) {
        throw std::invalid_argument("Goal target amount must be greater than 0.");
    }
    if (goal.currentAmount < 0.0) {
        throw std::invalid_argument("Goal current amount cannot be negative.");
    }
}

}  // namespace

// Creates a new goal and marks it complete if already reached.
models::Goal GoalService::createGoal(models::Goal goal) {
    validateGoal(goal);
    goal.id = nextId();
    goal.completed = goal.currentAmount >= goal.targetAmount;
    goals_.push_back(goal);
    return goal;
}

// Updates the editable fields for a goal.
models::Goal GoalService::updateGoal(const std::string& userId,
                                     const std::string& goalId,
                                     const models::Goal& updatedGoal) {
    models::Goal candidate = updatedGoal;
    candidate.id = goalId;
    candidate.userId = userId;
    validateGoal(candidate);
    candidate.completed = candidate.currentAmount >= candidate.targetAmount;

    for (auto& goal : goals_) {
        if (goal.id == goalId && goal.userId == userId) {
            goal = candidate;
            return goal;
        }
    }
    throw std::out_of_range("Goal not found.");
}

// Updates the current saved amount for a goal.
models::Goal GoalService::updateProgress(const std::string& userId,
                                         const std::string& goalId,
                                         double currentAmount) {
    if (currentAmount < 0.0) {
        throw std::invalid_argument("Goal current amount cannot be negative.");
    }
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

// Returns all incomplete goals for one user.
std::vector<models::Goal> GoalService::listActiveGoals(const std::string& userId) const {
    std::vector<models::Goal> result;
    for (const auto& goal : goals_) {
        if (goal.userId == userId && !goal.completed) {
            result.push_back(goal);
        }
    }
    return result;
}

// Returns all completed goals for one user.
std::vector<models::Goal> GoalService::listCompletedGoals(const std::string& userId) const {
    std::vector<models::Goal> result;
    for (const auto& goal : goals_) {
        if (goal.userId == userId && goal.completed) {
            result.push_back(goal);
        }
    }
    return result;
}

// Calculates completion ratio for progress bars.
double GoalService::progressRatio(const models::Goal& goal) {
    if (goal.targetAmount <= 0.0) {
        return 0.0;
    }
    return std::clamp(goal.currentAmount / goal.targetAmount, 0.0, 1.0);
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
