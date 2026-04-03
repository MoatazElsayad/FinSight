#pragma once

#include "Common.h"

#include <string>

namespace finsight::core::models {

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
