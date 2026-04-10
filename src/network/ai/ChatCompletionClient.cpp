#include "ChatCompletionClient.h"

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>

using namespace std;

namespace finsight::network::ai {

namespace {

#ifdef _WIN32
#define FINSIGHT_POPEN _popen
#define FINSIGHT_PCLOSE _pclose
#else
#define FINSIGHT_POPEN popen
#define FINSIGHT_PCLOSE pclose
#endif

// Reads the full stdout output of a shell command.
string readPipe(const string& command) {
    string output;
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

// Checks whether the configured API key has been replaced with a real value.
bool hasRealKey(const string& apiKey) {
    const bool isReal = !apiKey.empty() && apiKey != "PASTE_REAL_API_KEY_HERE" && apiKey.find("sk-or-v1") == 0;
    return isReal;
}

}  // namespace

// Sends one chat-completion request and parses the returned text.
core::models::AIChatResponse ChatCompletionClient::complete(const core::models::AIProviderConfig& config,
                                                            const core::models::AIChatRequest& request) const {
    core::models::AIChatResponse response {.model = request.model.empty() ? config.model : request.model};
    
    // Debug output
    cout << "AI Request - Model: " << response.model << ", API Key starts with: " << config.apiKey.substr(0, 10) << "..." << endl;
    
    if (!hasRealKey(config.apiKey)) {
        cout << "Using placeholder response - API key validation failed" << endl;
        response.success = true;
        response.usedFallback = true;
        response.content =
            "AI placeholder response. The API key appears to be invalid or not configured. "
            "Please check that FINSIGHT_OPENROUTER_API_KEY is set to a valid OpenRouter API key in your .env file.";
        return response;
    }

    const auto payload = buildPayload(request);
    const auto payloadPath = writeTempPayload(payload);

    ostringstream command;
    command << "curl -sS -X POST \"" << config.apiUrl << "\" "
            << "--max-time 20 "
            << "-H \"Authorization: Bearer " << config.apiKey << "\" "
            << "-H \"Content-Type: application/json\" "
            << "-H \"HTTP-Referer: " << config.appUrl << "\" "
            << "-H \"X-Title: " << config.appName << "\" "
            << "--data-binary @\"" << payloadPath << "\"";

    cout << "Executing curl command..." << endl;
    const auto raw = readPipe(command.str());
    cout << "Curl response length: " << raw.length() << endl;
    if (!raw.empty()) {
        cout << "Response starts with: " << raw.substr(0, 100) << "..." << endl;
    }
    deleteTempPayload(payloadPath);

    response.rawResponse = raw;
    response.content = extractContent(raw);
    response.success = !response.content.empty();
    if (!response.success) {
        response.usedFallback = true;
        response.error = "Could not parse model response. Check API URL, key, model, or response shape.";
        response.content =
            "AI request failed. Please check:\n"
            "- API key is valid and has credits\n"
            "- Model name is correct\n"
            "- Internet connection is working\n"
            "- OpenRouter service is available";
    }
    return response;
}

// Escapes raw text before it is inserted into JSON.
string ChatCompletionClient::escapeJson(const string& value) {
    string escaped;
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

// Builds the JSON request body for the chat API.
string ChatCompletionClient::buildPayload(const core::models::AIChatRequest& request) {
    ostringstream payload;
    payload << "{";
    payload << "\"model\":\"" << escapeJson(request.model) << "\",";
    payload << "\"temperature\":" << request.temperature << ",";
    payload << "\"messages\":[";
    for (size_t index = 0; index < request.messages.size(); ++index) {
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

// Extracts the assistant content field from the raw JSON response.
string ChatCompletionClient::extractContent(const string& responseBody) {
    const auto marker = string {"\"content\":\""};
    auto position = responseBody.find(marker);
    if (position == string::npos) {
        return {};
    }
    position += marker.size();

    string content;
    bool escaped = false;
    for (size_t index = position; index < responseBody.size(); ++index) {
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

// Writes the payload to a temporary file so curl can send it.
string ChatCompletionClient::writeTempPayload(const string& payload) {
    const auto path = (filesystem::temp_directory_path() / "finsight_ai_payload.json").string();
    ofstream output(path, ios::trunc);
    output << payload;
    return path;
}

// Removes the temporary payload file after the request finishes.
void ChatCompletionClient::deleteTempPayload(const string& path) {
    error_code error;
    filesystem::remove(path, error);
}

}  // namespace finsight::network::ai
