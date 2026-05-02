#pragma once
#include "IMessageHandler.h"
#include "core/managers/FinanceTrackerBackend.h"
#include <string>

class BackendMessageHandler : public IMessageHandler {
public:
    BackendMessageHandler(finsight::core::managers::FinanceTrackerBackend& backend);

    std::string handle(const std::string& jsonMessage) override;

private:
    finsight::core::managers::FinanceTrackerBackend& backend_;

    std::string getStr(std::string json, std::string key);
    std::string err(std::string message);
};
