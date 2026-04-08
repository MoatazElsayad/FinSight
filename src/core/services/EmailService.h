#pragma once

#include "../models/Email.h"

#include "../../network/email/ResendEmailClient.h"

using namespace std;

namespace finsight::core::services {

class EmailService {
public:
    // Stores the active email provider configuration.
    void configure(const models::EmailProviderConfig& config);
    // Returns the active email provider configuration.
    const models::EmailProviderConfig& config() const;
    // Sends one email through the configured provider.
    models::EmailSendResult sendEmail(const models::EmailMessage& message) const;

private:
    models::EmailProviderConfig config_ {};
    network::email::ResendEmailClient client_;
};

}  // namespace finsight::core::services
