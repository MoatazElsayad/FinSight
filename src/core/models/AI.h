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
    string apiKey;
    string model {"mistralai/mistral-small-3.1-24b-instruct:free"};
    vector<string> fallbackModels {
        "mistralai/mistral-small-3.1-24b-instruct:free",
        "meta-llama/llama-3.3-70b-instruct:free",
        "deepseek/deepseek-r1-0528:free",
        "qwen/qwen3-coder:free",
        "google/gemma-3-27b-it:free",
        "mistralai/mistral-7b-instruct:free",
        "qwen/qwen-2.5-vl-7b-instruct:free",
        "liquid/lfm-2.5-1.2b-instruct:free",
        "minimax/minimax-m2.5:free",
        "cognitivecomputations/dolphin-mistral-24b-venice-edition:free",
        "meta-llama/llama-3.2-3b-instruct:free"
    };
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
    int httpStatus {0};
};

// Holds AI-generated dashboard commentary.
struct AIDashboardInsight {
    string summary;
    vector<string> recommendations;
    bool usedFallback {false};
    string model;
    bool allModelsBusy {false};
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
