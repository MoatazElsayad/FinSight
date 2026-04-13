#pragma once

#include "../../core/models/AI.h"
#include "../client/HttpClient.h"

using namespace std;

namespace finsight::network::ai {

class ChatCompletionClient {
public:
    ChatCompletionClient();
    
    core::models::AIChatResponse complete(const core::models::AIProviderConfig& config,
                                          const core::models::AIChatRequest& request) const;

private:
    string escapeJson(const string& value) const;
    string buildPayload(const core::models::AIChatRequest& request) const;
    string extractContent(const string& responseBody) const;
    
    mutable finsight::network::client::HttpClient httpClient_;
};

}  // namespace finsight::network::ai