#pragma once
#include "IMessageHandler.h"
#include "core/managers/FinanceTrackerBackend.h"

class BackendMessageHandler : public IMessageHandler {
public:
    explicit BackendMessageHandler(
        finsight::core::managers::FinanceTrackerBackend& backend);
  std::string handle(const std::string& jsonMessage) override;

private:
    finsight::core::managers::FinanceTrackerBackend& backend_;
    static std::string getStr(const std::string& json, const std::string& key);
    static std::string err(const std::string& message);
};
