#pragma once

#include "../models/Session.h"

#include <cstddef>
#include <optional>
#include <string>
#include <vector>

namespace finsight::core::services {

class SessionService {
public:
    models::Session startSession(const std::string& userId, const models::Date& issuedOn);
    void endSession(const std::string& token);
    std::optional<models::Session> getSession(const std::string& token) const;
    std::vector<models::Session> sessionsForUser(const std::string& userId) const;
    std::vector<models::Session> allSessions() const;
    void loadSessions(std::vector<models::Session> sessions);

private:
    std::string nextToken();

    std::vector<models::Session> sessions_;
    std::size_t nextSessionId_ {1};
};

}  // namespace finsight::core::services
