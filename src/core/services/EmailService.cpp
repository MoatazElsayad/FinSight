#include "EmailService.h"

using namespace std;

namespace finsight::core::services {

// Stores the email configuration used by later send requests.
void EmailService::configure(const models::EmailProviderConfig& config) {
    config_ = config;
}

// Returns the email configuration used by this service.
const models::EmailProviderConfig& EmailService::config() const {
    return config_;
}

// Sends one email message using the configured email client.
models::EmailSendResult EmailService::sendEmail(const models::EmailMessage& message) const {
    return client_.send(config_, message);
}

}  // namespace finsight::core::services
