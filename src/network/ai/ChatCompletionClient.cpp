#include "ChatCompletionClient.h"

#include <sstream>
#include <string>

using namespace std;

namespace finsight::network::ai {

ChatCompletionClient::ChatCompletionClient() {
    httpClient_.setTimeout(45);
}

bool hasRealKey(const string& apiKey) {
    return !apiKey.empty() && apiKey.find("sk-or-v1") == 0;
}

core::models::AIChatResponse ChatCompletionClient::complete(const core::models::AIProviderConfig& config,
                                                            const core::models::AIChatRequest& request) const {
    core::models::AIChatResponse response {.model = request.model.empty() ? config.model : request.model};
    
    if (!hasRealKey(config.apiKey)) {
        response.success = true;
        response.usedFallback = true;
        response.content =
            "AI placeholder response. The API key appears to be invalid or not configured. "
            "Please set OPENROUTER_API_KEY to a valid OpenRouter API key in your .env file.";
        return response;
    }
    
    auto payload = buildPayload(request);
    
    finsight::network::protocol::HttpRequest httpRequest;
    httpRequest.method = finsight::network::protocol::HttpMethod::Post;
    httpRequest.url = config.apiUrl;
    httpRequest.body = payload;
    httpRequest.timeoutSeconds = 45;
    httpRequest.headers.insert(std::make_pair("Authorization", "Bearer " + config.apiKey));
    httpRequest.headers.insert(std::make_pair("Content-Type", "application/json"));
    httpRequest.headers.insert(std::make_pair("HTTP-Referer", config.appUrl));
    httpRequest.headers.insert(std::make_pair("X-Title", config.appName));
    
    httpClient_.setTimeout(45);
    auto result = httpClient_.sendRequest(httpRequest);
    
    response.rawResponse = result.response.body;
    response.httpStatus = result.response.statusCode;
    response.success = result.response.success;
    
    if (response.success) {
        response.content = extractContent(result.response.body);
        response.success = !response.content.empty();
    }
    
    if (!response.success) {
        response.usedFallback = true;
        string transportError = result.response.error.empty() ? string {} : result.response.error;
        if (response.httpStatus != 0) {
            response.error = transportError.empty()
                                 ? ("HTTP " + to_string(response.httpStatus))
                                 : ("HTTP " + to_string(response.httpStatus) + ": " + transportError);
        } else {
            response.error = transportError.empty() ? "Request failed" : transportError;
        }
        response.content.clear();
    }
    
    return response;
}

string ChatCompletionClient::escapeJson(const string& value) const {
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

string ChatCompletionClient::buildPayload(const core::models::AIChatRequest& request) const {
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
    payload << "]}";
    return payload.str();
}

string ChatCompletionClient::extractContent(const string& responseBody) const {
    auto decodeFrom = [&](size_t position) -> string {
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
    };

    const string assistantAnchor = "\"role\":\"assistant\"";
    size_t searchFrom = 0;
    const size_t asst = responseBody.find(assistantAnchor);
    if (asst != string::npos) {
        searchFrom = asst;
    }

    const string marker = "\"content\":\"";
    auto position = responseBody.find(marker, searchFrom);
    if (position == string::npos) {
        const string spaced = "\"content\": \"";
        position = responseBody.find(spaced, searchFrom);
        if (position == string::npos) {
            return {};
        }
        position += spaced.size();
    } else {
        position += marker.size();
    }

    return decodeFrom(position);
}

}  // namespace finsight::network::ai