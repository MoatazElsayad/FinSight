#include "ResendEmailClient.h"

#include <sstream>

using namespace std;

namespace finsight::network::email {

ResendEmailClient::ResendEmailClient() {
    httpClient_.setTimeout(10);
}

bool hasRealKey(const string& apiKey) {
    return apiKey.rfind("re_", 0) == 0 && apiKey != "PASTE_REAL_RESEND_API_KEY_HERE";
}

core::models::EmailSendResult ResendEmailClient::send(const core::models::EmailProviderConfig& config,
                                                      const core::models::EmailMessage& message) const {
    core::models::EmailSendResult result {
        .attempted = false,
        .success = false,
        .recipient = message.to,
        .subject = message.subject,
    };

    if (!config.enabled) {
        result.error = "Email sending is disabled in EmailProviderConfig.";
        return result;
    }
    if (config.apiUrl.empty() || config.fromEmail.empty()) {
        result.error = "Resend configuration is incomplete.";
        return result;
    }
    if (!hasRealKey(config.apiKey)) {
        result.error = "Resend API key is missing or still set to the placeholder value.";
        return result;
    }
    if (message.to.empty() || message.subject.empty() || message.body.empty()) {
        result.error = "Email message is missing recipient, subject, or body.";
        return result;
    }

    const string payload = buildPayload(config, message);

    finsight::network::protocol::HttpRequest httpRequest;
    httpRequest.method = finsight::network::protocol::HttpMethod::Post;
    httpRequest.url = config.apiUrl;
    httpRequest.body = payload;
    httpRequest.timeoutSeconds = 10;
    httpRequest.headers.insert(std::make_pair("Authorization", "Bearer " + config.apiKey));
    httpRequest.headers.insert(std::make_pair("Content-Type", "application/json"));

    result.attempted = true;
    auto httpResult = httpClient_.sendRequest(httpRequest);
    
    result.providerResponse = httpResult.response.body;
    result.success = httpResult.response.success && 
                  httpResult.response.body.find("\"id\":") != string::npos &&
                  httpResult.response.body.find("\"message\"") == string::npos;
    if (!result.success) {
        result.error = !httpResult.response.error.empty()
                           ? httpResult.response.error
                           : (httpResult.response.body.empty() ? "Request failed" : httpResult.response.body);
    }

    return result;
}

string ResendEmailClient::escapeJson(const string& value) const {
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

string ResendEmailClient::buildPayload(const core::models::EmailProviderConfig& config,
                                       const core::models::EmailMessage& message) const {
    const string sender = config.fromName.empty()
                              ? config.fromEmail
                              : config.fromName + " <" + config.fromEmail + ">";

    ostringstream payload;
    payload << "{";
    payload << "\"from\":\"" << escapeJson(sender) << "\",";
    payload << "\"to\":\"" << escapeJson(message.to) << "\",";
    payload << "\"subject\":\"" << escapeJson(message.subject) << "\",";
    payload << "\"text\":\"" << escapeJson(message.body) << "\"";
    if (!message.htmlBody.empty()) {
        payload << ",\"html\":\"" << escapeJson(message.htmlBody) << "\"";
    }
    payload << "}";
    return payload.str();
}

}  // namespace finsight::network::email
