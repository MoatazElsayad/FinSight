#pragma once

#include "Common.h"
#include "Receipt.h"

#include <string>
#include <vector>

using namespace std;

namespace finsight::core::models {

// Stores the provider settings used by the AI client.
struct AIProviderConfig {
    string apiUrl {"https://openrouter.ai/api/v1/chat/completions"};
    string apiKey {"PASTE_REAL_API_KEY_HERE"};
    string model {"openrouter/free"};
    vector<string> fallbackModels;
    string appName {"FinSight"};
    string appUrl {"https://example.com/finsight"};
};

// Represents one chat message sent to the model.
struct AIMessage {
    string role;
    string content;
};

// Describes a chat completion request.
struct AIChatRequest {
    string model;
    vector<AIMessage> messages;
    double temperature {0.2};
};

// Captures the model response and fallback metadata.
struct AIChatResponse {
    bool success {false};
    string model;
    string content;
    string rawResponse;
    string error;
    bool usedFallback {false};
    vector<string> attemptedModels;
};

// Holds AI-generated dashboard commentary.
struct AIDashboardInsight {
    string summary;
    vector<string> recommendations;
    bool usedFallback {false};
};

// Holds AI-generated savings advice.
struct AISavingsInsight {
    string summary;
    vector<string> actions;
    bool usedFallback {false};
};

// Holds the answer for a finance chat question.
struct AIFinanceChatAnswer {
    string answer;
    bool usedFallback {false};
};

// Holds AI-assisted receipt parsing suggestions.
struct AIReceiptSuggestion {
    ReceiptParseResult parseResult;
    bool usedFallback {false};
};

}  // namespace finsight::core::models
