#include "SessionService.h"

#include <regex>
#include <sstream>

namespace finsight::core::services {

models::Session SessionService::startSession(const std::string& userId, const models::Date& issuedOn) {
    models::Session session {
        .token = nextToken(),
        .userId = userId,
        .issuedOn = issuedOn,
        .active = true,
    };
    sessions_.push_back(session);
    return session;
}

void SessionService::endSession(const std::string& token) {
    for (auto& session : sessions_) {
        if (session.token == token) {
            session.active = false;
            return;
        }
    }
}

std::optional<models::Session> SessionService::getSession(const std::string& token) const {
    for (const auto& session : sessions_) {
        if (session.token == token) {
            return session;
        }
    }
    return std::nullopt;
}

std::vector<models::Session> SessionService::sessionsForUser(const std::string& userId) const {
    std::vector<models::Session> result;
    for (const auto& session : sessions_) {
        if (session.userId == userId) {
            result.push_back(session);
        }
    }
    return result;
}

std::vector<models::Session> SessionService::allSessions() const {
    return sessions_;
}

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

std::string SessionService::nextToken() {
    std::ostringstream stream;
    stream << "session-" << nextSessionId_++;
    return stream.str();
}

}  // namespace finsight::core::services
