#pragma once

#include "../../core/models/Email.h"
#include "../client/HttpClient.h"

using namespace std;

namespace finsight::network::email {

class ResendEmailClient {
public:
    ResendEmailClient();
    
    core::models::EmailSendResult send(const core::models::EmailProviderConfig& config,
                                       const core::models::EmailMessage& message) const;

private:
    string escapeJson(const string& value) const;
    string buildPayload(const core::models::EmailProviderConfig& config,
                         const core::models::EmailMessage& message) const;
    
    mutable finsight::network::client::HttpClient httpClient_;
};

}  // namespace finsight::network::email