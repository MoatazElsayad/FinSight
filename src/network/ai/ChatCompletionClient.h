#pragma once

#include "../../core/models/AI.h"

namespace finsight::network::ai {

class ChatCompletionClient {
public:
    core::models::AIChatResponse complete(const core::models::AIProviderConfig& config,
                                          const core::models::AIChatRequest& request) const;

private:
    static std::string escapeJson(const std::string& value);
    static std::string buildPayload(const core::models::AIChatRequest& request);
    static std::string extractContent(const std::string& responseBody);
    static std::string writeTempPayload(const std::string& payload);
    static void deleteTempPayload(const std::string& path);
};

}  // namespace finsight::network::ai
