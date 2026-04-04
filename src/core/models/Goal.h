#pragma once

#include "Common.h"

#include <string>

using namespace std;

namespace finsight::core::models {

// Represents a long-term finance target the user is tracking.
struct Goal {
    std::string id;
    std::string userId;
    std::string title;
    std::string description;
    double targetAmount {0.0};
    double currentAmount {0.0};
    Date targetDate;
    bool completed {false};
};

}  // namespace finsight::core::models
