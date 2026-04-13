#include "AuthService.h"

#include "../models/Common.h"

#include <functional>
#include <regex>
#include <sstream>
#include <stdexcept>

using namespace std;

namespace finsight::core::services {

// Registers a new user after checking for duplicate emails.
models::User AuthService::registerUser(const std::string& fullName,
                                       const std::string& email,
                                       const std::string& phone,
                                       const std::string& gender,
                                       const std::string& password,
                                       const models::Date& createdAt) {
    if (fullName.empty() || email.empty() || password.empty()) {
        throw std::invalid_argument("Full name, email, and password are required.");
    }
    for (const auto& user : users_) {
        if (models::toLower(user.email) == models::toLower(email)) {
            throw std::invalid_argument("Email is already registered.");
        }
    }

    models::User user {
        .id = nextId(),
        .fullName = fullName,
        .email = email,
        .phone = phone,
        .gender = gender,
        .passwordHash = hashPassword(password),
        .createdAt = createdAt,
    };
    users_.push_back(user);
    return user;
}

// Tries to authenticate a user by email and password.
std::optional<models::User> AuthService::login(const std::string& email, const std::string& password) {
    const auto hashedPassword = hashPassword(password);
    const auto legacyPassword = legacyHashPassword(password);
    for (auto& user : users_) {
        if (models::toLower(user.email) != models::toLower(email)) {
            continue;
        }
        if (user.passwordHash == hashedPassword || user.passwordHash == legacyPassword) {
            return user;
        }
        // Seeded or legacy databases sometimes store the raw password in password_hash.
        if (user.passwordHash == password) {
            user.passwordHash = hashedPassword;
            return user;
        }
    }
    return std::nullopt;
}

// Updates the editable profile fields for one stored user.
models::User AuthService::updateProfile(const std::string& userId,
                                        const std::string& fullName,
                                        const std::string& phone,
                                        const std::string& gender) {
    for (auto& user : users_) {
        if (user.id == userId) {
            user.fullName = fullName;
            user.phone = phone;
            user.gender = gender;
            return user;
        }
    }
    throw std::out_of_range("User not found.");
}

// Returns a stored user by id.
const models::User& AuthService::getUser(const std::string& userId) const {
    for (const auto& user : users_) {
        if (user.id == userId) {
            return user;
        }
    }
    throw std::out_of_range("User not found.");
}

// Returns all stored users.
std::vector<models::User> AuthService::listUsers() const {
    return users_;
}

// Restores users from persisted state and resets id generation.
void AuthService::loadUsers(std::vector<models::User> users) {
    users_ = std::move(users);
    std::size_t maxId = 0;
    const std::regex pattern(R"(usr-(\d+))");
    for (const auto& user : users_) {
        std::smatch match;
        if (std::regex_match(user.id, match, pattern)) {
            maxId = std::max<std::size_t>(maxId, static_cast<std::size_t>(std::stoull(match[1].str())));
        }
    }
    nextUserId_ = maxId + 1;
}

// Builds the next user id string.
std::string AuthService::nextId() {
    std::ostringstream stream;
    stream << "usr-" << nextUserId_++;
    return stream.str();
}

// Legacy hash used by prior versions.
std::string AuthService::legacyHashPassword(const std::string& password) {
    return std::to_string(std::hash<std::string> {}(password));
}

// Creates a simple hash string for a password.
std::string AuthService::hashPassword(const std::string& password) {
    // Use DJB2 hash algorithm for deterministic hashing
    // This ensures the same password always produces the same hash
    unsigned long hash = 5381;
    for (unsigned char c : password) {
        hash = ((hash << 5) + hash) + c;
    }
    return std::to_string(hash);
}

}  // namespace finsight::core::services
