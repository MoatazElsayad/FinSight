#pragma once
#include <string>

// GMock.
class IMessageHandler {
public:
    virtual ~IMessageHandler() = default;
    virtual std::string handle(const std::string& jsonMessage) = 0;
};
