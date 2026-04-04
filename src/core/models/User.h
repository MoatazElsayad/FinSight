#pragma once

#include "Common.h"

#include <string>

using namespace std;

namespace finsight::core::models {

// Stores the basic profile information for an application user.
struct User {
    std::string id;
    std::string fullName;
    std::string email;
    std::string phone;
    std::string gender;
    std::string passwordHash;
    Date createdAt;
};

}  // namespace finsight::core::models
