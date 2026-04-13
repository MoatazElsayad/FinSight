#pragma once

#include <string>
#include <vector>
#include <unordered_map>

namespace finsight::network::protocol {

enum class HttpMethod {
    Get,
    Post,
    Put,
    Delete
};

struct HttpRequest {
    HttpMethod method;
    std::string url;
    std::string path;
    std::unordered_map<std::string, std::string> headers;
    std::string body;
    int timeoutSeconds {20};
};

struct HttpResponse {
    int statusCode {0};
    std::string statusMessage;
    std::unordered_map<std::string, std::string> headers;
    std::string body;
    bool success {false};
    std::string error;
};

struct HttpResult {
    bool attempted {false};
    HttpResponse response;
};

}  // namespace finsight::network::protocol