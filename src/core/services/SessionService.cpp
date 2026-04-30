#include "SessionService.h"

#include <regex>
#include <sstream>
#include <stdexcept>

using namespace std;

namespace finsight::core::services {

// Starts and stores a new user session.
models::Session SessionService::startSession(const std::string& userId, const models::Date& issuedOn) {
    if (userId.empty()) {
        throw std::invalid_argument("Session requires a user.");
    }
    models::Session session {
        .token = nextToken(),
        .userId = userId,
        .issuedOn = issuedOn,
        .active = true,
    };
    sessions_.push_back(session);
    return session;
}

// Marks a stored session as inactive.
void SessionService::endSession(const std::string& token) {
    for (auto& session : sessions_) {
        if (session.token == token) {
            session.active = false;
            return;
        }
    }
}

// Returns one session by token if it exists.
std::optional<models::Session> SessionService::getSession(const std::string& token) const {
    for (const auto& session : sessions_) {
        if (session.token == token) {
            return session;
        }
    }
    return std::nullopt;
}

// Checks whether a session token exists and is still active.
bool SessionService::isSessionActive(const std::string& token) const {
    const auto session = getSession(token);
    return session.has_value() && session->active;
}

// Returns all sessions belonging to one user.
std::vector<models::Session> SessionService::sessionsForUser(const std::string& userId) const {
    std::vector<models::Session> result;
    for (const auto& session : sessions_) {
        if (session.userId == userId) {
            result.push_back(session);
        }
    }
    return result;
}

// Returns every stored session.
std::vector<models::Session> SessionService::allSessions() const {
    return sessions_;
}

// Restores sessions from persisted state and resets token generation.
void SessionService::loadSessions(std::vector<models::Session> sessions) {
    sessions_ = std::move(sessions);
    std::size_t maxId = 0;
    const std::regex pattern(R"(session-(\d+))");
    for (const auto& session : sessions_) {
        std::smatch match;
        if (std::regex_match(session.token, match, pattern)) {
            maxId = std::max<std::size_t>(maxId, static_cast<std::size_t>(std::stoull(match[1].str())));
        }
    }
    nextSessionId_ = maxId + 1;
}

// Builds the next session token string.
std::string SessionService::nextToken() {
    std::ostringstream stream;
    stream << "session-" << nextSessionId_++;
    return stream.str();
}

}  // namespace finsight::core::services
