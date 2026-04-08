#pragma once

#include "../../core/models/Email.h"

using namespace std;

namespace finsight::network::email {

class ResendEmailClient {
public:
    // Sends one email through the Resend HTTP API.
    core::models::EmailSendResult send(const core::models::EmailProviderConfig& config,
                                       const core::models::EmailMessage& message) const;

private:
    // Escapes raw text before it is embedded into JSON.
    static string escapeJson(const string& value);
    // Builds the JSON payload expected by the Resend email API.
    static string buildPayload(const core::models::EmailProviderConfig& config,
                               const core::models::EmailMessage& message);
    // Writes the JSON payload to a temporary file for curl to send.
    static string writeTempPayload(const string& payload);
    // Removes the temporary payload file after the request finishes.
    static void deleteTempPayload(const string& path);
};

}  // namespace finsight::network::email
