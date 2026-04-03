#include "ChatCompletionClient.h"

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <sstream>

namespace finsight::network::ai {

namespace {

#ifdef _WIN32
#define FINSIGHT_POPEN _popen
#define FINSIGHT_PCLOSE _pclose
#else
#define FINSIGHT_POPEN popen
#define FINSIGHT_PCLOSE pclose
#endif

std::string readPipe(const std::string& command) {
    std::string output;
    FILE* pipe = FINSIGHT_POPEN(command.c_str(), "r");
    if (pipe == nullptr) {
        return {};
    }

    char buffer[256];
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        output += buffer;
    }
    FINSIGHT_PCLOSE(pipe);
    return output;
}

bool hasRealKey(const std::string& apiKey) {
    return !apiKey.empty() && apiKey != "PASTE_REAL_API_KEY_HERE";
}

}  // namespace

core::models::AIChatResponse ChatCompletionClient::complete(const core::models::AIProviderConfig& config,
                                                            const core::models::AIChatRequest& request) const {
    core::models::AIChatResponse response {.model = request.model.empty() ? config.model : request.model};
    if (!hasRealKey(config.apiKey)) {
        response.success = true;
        response.usedFallback = true;
        response.content =
            "AI placeholder response. Replace the API key and model in AIProviderConfig to use a real provider.";
        return response;
    }

    const auto payload = buildPayload(request);
    const auto payloadPath = writeTempPayload(payload);

    std::ostringstream command;
    command << "curl -sS -X POST \"" << config.apiUrl << "\" "
            << "--max-time 20 "
            << "-H \"Authorization: Bearer " << config.apiKey << "\" "
            << "-H \"Content-Type: application/json\" "
            << "-H \"HTTP-Referer: " << config.appUrl << "\" "
            << "-H \"X-Title: " << config.appName << "\" "
            << "--data-binary @\"" << payloadPath << "\"";

    const auto raw = readPipe(command.str());
    deleteTempPayload(payloadPath);

    response.rawResponse = raw;
    response.content = extractContent(raw);
    response.success = !response.content.empty();
    if (!response.success) {
        response.usedFallback = true;
        response.error = "Could not parse model response. Check API URL, key, model, or response shape.";
        response.content =
            "AI request reached the provider but the response could not be parsed by the current client.";
    }
    return response;
}

std::string ChatCompletionClient::escapeJson(const std::string& value) {
    std::string escaped;
    for (char ch : value) {
        switch (ch) {
        case '\\':
            escaped += "\\\\";
            break;
        case '"':
            escaped += "\\\"";
            break;
        case '\n':
            escaped += "\\n";
            break;
        case '\r':
            escaped += "\\r";
            break;
        case '\t':
            escaped += "\\t";
            break;
        default:
            escaped.push_back(ch);
            break;
        }
    }
    return escaped;
}

std::string ChatCompletionClient::buildPayload(const core::models::AIChatRequest& request) {
    std::ostringstream payload;
    payload << "{";
    payload << "\"model\":\"" << escapeJson(request.model) << "\",";
    payload << "\"temperature\":" << request.temperature << ",";
    payload << "\"messages\":[";
    for (std::size_t index = 0; index < request.messages.size(); ++index) {
        if (index > 0) {
            payload << ",";
        }
        payload << "{"
                << "\"role\":\"" << escapeJson(request.messages[index].role) << "\","
                << "\"content\":\"" << escapeJson(request.messages[index].content) << "\""
                << "}";
    }
    payload << "]";
    payload << "}";
    return payload.str();
}

std::string ChatCompletionClient::extractContent(const std::string& responseBody) {
    const auto marker = std::string {"\"content\":\""};
    auto position = responseBody.find(marker);
    if (position == std::string::npos) {
        return {};
    }
    position += marker.size();

    std::string content;
    bool escaped = false;
    for (std::size_t index = position; index < responseBody.size(); ++index) {
        const char ch = responseBody[index];
        if (escaped) {
            switch (ch) {
            case 'n':
                content.push_back('\n');
                break;
            case 'r':
                content.push_back('\r');
                break;
            case 't':
                content.push_back('\t');
                break;
            default:
                content.push_back(ch);
                break;
            }
            escaped = false;
            continue;
        }
        if (ch == '\\') {
            escaped = true;
            continue;
        }
        if (ch == '"') {
            break;
        }
        content.push_back(ch);
    }
    return content;
}

std::string ChatCompletionClient::writeTempPayload(const std::string& payload) {
    const auto path = (std::filesystem::temp_directory_path() / "finsight_ai_payload.json").string();
    std::ofstream output(path, std::ios::trunc);
    output << payload;
    return path;
}

void ChatCompletionClient::deleteTempPayload(const std::string& path) {
    std::error_code error;
    std::filesystem::remove(path, error);
}

}  // namespace finsight::network::ai
