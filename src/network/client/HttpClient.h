#pragma once

#include "../protocol/Message.h"

#include <boost/asio.hpp>
#include <string>
#include <memory>

namespace finsight::network::client {

class HttpClient {
public:
    HttpClient();
    explicit HttpClient(int timeoutSeconds);
    ~HttpClient();

    finsight::network::protocol::HttpResult sendRequest(const finsight::network::protocol::HttpRequest& request);

    void setTimeout(int seconds);
    void setHeader(const std::string& key, const std::string& value);
    void clearHeaders();

private:
    bool parseUrl(const std::string& url, std::string& host, std::string& port, std::string& path);
    std::string buildHttpRequest(const finsight::network::protocol::HttpRequest& request);
    finsight::network::protocol::HttpResponse parseHttpResponse(const std::string& raw);

    int timeoutSeconds_;
    std::unordered_map<std::string, std::string> defaultHeaders_;
};

}  // namespace finsight::network::client