#include "ResendEmailClient.h"

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <sstream>

using namespace std;

namespace finsight::network::email {

namespace {

#ifdef _WIN32
#define FINSIGHT_POPEN _popen
#define FINSIGHT_PCLOSE _pclose
#else
#define FINSIGHT_POPEN popen
#define FINSIGHT_PCLOSE pclose
#endif

// Stores the stdout text and exit code from one shell command.
struct CommandResult {
    int exitCode {0};
    string output;
};

// Executes one shell command and captures its text output.
CommandResult runCommand(const string& command) {
    CommandResult result;
    FILE* pipe = FINSIGHT_POPEN(command.c_str(), "r");
    if (pipe == nullptr) {
        result.exitCode = -1;
        result.output = "Could not start curl process.";
        return result;
    }

    char buffer[256];
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        result.output += buffer;
    }
    result.exitCode = FINSIGHT_PCLOSE(pipe);
    return result;
}

// Checks whether the configured API key has been replaced with a real value.
bool hasRealKey(const string& apiKey) {
    return !apiKey.empty() && apiKey != "PASTE_REAL_RESEND_API_KEY_HERE";
}

}  // namespace

// Sends one email using the Resend HTTP API through curl.
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
    const string payloadPath = writeTempPayload(payload);

    ostringstream command;
    command << "curl -sS -X POST \"" << config.apiUrl << "\" "
            << "--max-time 20 "
            << "-H \"Authorization: Bearer " << config.apiKey << "\" "
            << "-H \"Content-Type: application/json\" "
            << "--data-binary @\"" << payloadPath << "\" 2>&1";

    result.attempted = true;
    const auto commandResult = runCommand(command.str());
    result.providerResponse = commandResult.output;
    result.success = commandResult.exitCode == 0 &&
                     commandResult.output.find("\"id\":") != string::npos &&
                     commandResult.output.find("\"message\"") == string::npos;
    if (!result.success) {
        result.error = commandResult.output.empty()
                           ? "curl failed to send the email through the Resend API."
                           : commandResult.output;
    }

    deleteTempPayload(payloadPath);
    return result;
}

// Escapes raw text before it is inserted into JSON.
string ResendEmailClient::escapeJson(const string& value) {
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

// Builds the JSON request body expected by Resend.
string ResendEmailClient::buildPayload(const core::models::EmailProviderConfig& config,
                                       const core::models::EmailMessage& message) {
    ostringstream payload;
    payload << "{";
    payload << "\"from\":\"" << escapeJson(config.fromEmail) << "\",";
    payload << "\"to\":\"" << escapeJson(message.to) << "\",";
    payload << "\"subject\":\"" << escapeJson(message.subject) << "\",";
    payload << "\"html\":\"" << escapeJson(message.body) << "\"";
    payload << "}";
    return payload.str();
}

// Writes the payload to a temporary file so curl can send it.
string ResendEmailClient::writeTempPayload(const string& payload) {
    const auto path = (filesystem::temp_directory_path() / "finsight_email_payload.json").string();
    ofstream output(path, ios::trunc);
    output << payload;
    return path;
}

// Removes the temporary payload file after the request finishes.
void ResendEmailClient::deleteTempPayload(const string& path) {
    error_code error;
    filesystem::remove(path, error);
}

}  // namespace finsight::network::email
