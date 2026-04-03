#pragma once

#include "../models/User.h"

#include <cstddef>
#include <optional>
#include <string>
#include <vector>

namespace finsight::core::services {

class AuthService {
public:
    models::User registerUser(const std::string& fullName,
                              const std::string& email,
                              const std::string& phone,
                              const std::string& gender,
                              const std::string& password,
                              const models::Date& createdAt);
    std::optional<models::User> login(const std::string& email, const std::string& password) const;
    models::User updateProfile(const std::string& userId,
                               const std::string& fullName,
                               const std::string& phone,
                               const std::string& gender);
    const models::User& getUser(const std::string& userId) const;
    std::vector<models::User> listUsers() const;
    void loadUsers(std::vector<models::User> users);

private:
    std::string nextId();
    static std::string hashPassword(const std::string& password);

    std::vector<models::User> users_;
    std::size_t nextUserId_ {1};
};

}  // namespace finsight::core::services
