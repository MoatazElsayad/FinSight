#pragma once

#include <string>

using namespace std;

namespace finsight::core::models {

// Stores Resend API configuration used for sending application emails.
struct EmailProviderConfig {
    bool enabled {false};
    string apiUrl {"https://api.resend.com/emails"};
    string apiKey;
    string fromEmail;
    string fromName {"FinSight"};
};

// Stores one outgoing email message.
struct EmailMessage {
    string to;
    string subject;
    string body;
};

// Stores the result of one send attempt.
struct EmailSendResult {
    bool attempted {false};
    bool success {false};
    string recipient;
    string subject;
    string providerResponse;
    string error;
};

}  // namespace finsight::core::models
