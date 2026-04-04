#pragma once

#include "Common.h"

#include <string>

using namespace std;

namespace finsight::core::models {

// Represents a simple authenticated user session.
struct Session {
    std::string token;
    std::string userId;
    Date issuedOn;
    bool active {true};
};

}  // namespace finsight::core::models
