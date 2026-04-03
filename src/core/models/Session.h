#pragma once

#include "Common.h"

#include <string>

namespace finsight::core::models {

struct Session {
    std::string token;
    std::string userId;
    Date issuedOn;
    bool active {true};
};

}  // namespace finsight::core::models
