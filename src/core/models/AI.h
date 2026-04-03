#pragma once

#include "Common.h"
#include "Receipt.h"

#include <string>
#include <vector>

namespace finsight::core::models {

struct AIProviderConfig {
    std::string apiUrl {"https://openrouter.ai/api/v1/chat/completions"};
    std::string apiKey {"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"};
    std::string model {"openrouter/free"};
    std::vector<std::string> fallbackModels;
    std::string appName {"FinSight"};
    std::string appUrl {"https://example.com/finsight"};
};

struct AIMessage {
    std::string role;
    std::string content;
};

struct AIChatRequest {
    std::string model;
    std::vector<AIMessage> messages;
    double temperature {0.2};
};

struct AIChatResponse {
    bool success {false};
    std::string model;
    std::string content;
    std::string rawResponse;
    std::string error;
    bool usedFallback {false};
    std::vector<std::string> attemptedModels;
};

struct AIDashboardInsight {
    std::string summary;
    std::vector<std::string> recommendations;
    bool usedFallback {false};
};

struct AISavingsInsight {
    std::string summary;
    std::vector<std::string> actions;
    bool usedFallback {false};
};

struct AIFinanceChatAnswer {
    std::string answer;
    bool usedFallback {false};
};

struct AIReceiptSuggestion {
    ReceiptParseResult parseResult;
    bool usedFallback {false};
};

}  // namespace finsight::core::models
