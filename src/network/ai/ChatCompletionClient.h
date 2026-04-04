#pragma once

#include "../../core/models/AI.h"

using namespace std;

namespace finsight::network::ai {

class ChatCompletionClient {
public:
    // Sends one chat completion request to the configured AI provider.
    core::models::AIChatResponse complete(const core::models::AIProviderConfig& config,
                                          const core::models::AIChatRequest& request) const;

private:
    // Escapes raw text before it is embedded into JSON.
    static string escapeJson(const string& value);
    // Builds the request payload expected by chat-completions APIs.
    static string buildPayload(const core::models::AIChatRequest& request);
    // Extracts the returned assistant text from the raw JSON response.
    static string extractContent(const string& responseBody);
    // Writes the JSON payload to a temporary file for curl to send.
    static string writeTempPayload(const string& payload);
    // Removes the temporary payload file after the request finishes.
    static void deleteTempPayload(const string& path);
};

}  // namespace finsight::network::ai
