#pragma once

#include "../models/User.h"

#include <cstddef>
#include <optional>
#include <string>
#include <vector>

using namespace std;

namespace finsight::core::services {

class AuthService {
public:
    // Registers a new user and stores the profile in memory.
    models::User registerUser(const std::string& fullName,
                              const std::string& email,
                              const std::string& phone,
                              const std::string& gender,
                              const std::string& password,
                              const models::Date& createdAt);
    // Validates credentials and returns the matching user if found.
    std::optional<models::User> login(const std::string& email, const std::string& password) const;
    // Updates the editable profile fields for one user.
    models::User updateProfile(const std::string& userId,
                               const std::string& fullName,
                               const std::string& phone,
                               const std::string& gender);
    // Returns one user by id or throws if it does not exist.
    const models::User& getUser(const std::string& userId) const;
    // Returns all registered users.
    std::vector<models::User> listUsers() const;
    // Replaces the stored users from persisted data.
    void loadUsers(std::vector<models::User> users);

private:
    // Generates the next user id.
    std::string nextId();
    // Creates a simple deterministic hash for stored passwords.
    static std::string hashPassword(const std::string& password);

    std::vector<models::User> users_;
    std::size_t nextUserId_ {1};
};

}  // namespace finsight::core::services
