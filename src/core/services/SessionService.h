#pragma once

#include "../models/Session.h"

#include <cstddef>
#include <optional>
#include <string>
#include <vector>

using namespace std;

namespace finsight::core::services {

class SessionService {
public:
    // Starts a new user session and returns its token.
    models::Session startSession(const std::string& userId, const models::Date& issuedOn);
    // Marks a session as inactive.
    void endSession(const std::string& token);
    // Returns one session by token if it exists.
    std::optional<models::Session> getSession(const std::string& token) const;
    // Returns all sessions created by a user.
    std::vector<models::Session> sessionsForUser(const std::string& userId) const;
    // Returns every stored session.
    std::vector<models::Session> allSessions() const;
    // Replaces the stored sessions from persisted data.
    void loadSessions(std::vector<models::Session> sessions);

private:
    // Generates the next session token.
    std::string nextToken();

    std::vector<models::Session> sessions_;
    std::size_t nextSessionId_ {1};
};

}  // namespace finsight::core::services
