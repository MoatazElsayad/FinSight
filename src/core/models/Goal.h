#pragma once

#include "Common.h"

#include <string>

namespace finsight::core::models {

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
