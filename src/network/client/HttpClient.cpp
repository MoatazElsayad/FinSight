#include "HttpClient.h"

#include <algorithm>
#include <sstream>

#include <QByteArray>
#include <QEventLoop>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QString>
#include <QTimer>
#include <QUrl>

namespace finsight::network::client {

namespace {

finsight::network::protocol::HttpResult sendHttpsViaQt(
    const finsight::network::protocol::HttpRequest& request,
    int defaultTimeoutSeconds,
    const std::unordered_map<std::string, std::string>& defaultHeaders) {
    using namespace finsight::network::protocol;

    HttpResult result;
    result.attempted = true;

    const QUrl url(QString::fromStdString(request.url));
    if (!url.isValid() || url.scheme() != QStringLiteral("https")) {
        result.response.error = "Invalid HTTPS URL";
        return result;
    }

    QNetworkRequest qreq(url);
    for (const auto& header : defaultHeaders) {
        qreq.setRawHeader(QByteArray::fromStdString(header.first), QByteArray::fromStdString(header.second));
    }
    for (const auto& header : request.headers) {
        qreq.setRawHeader(QByteArray::fromStdString(header.first), QByteArray::fromStdString(header.second));
    }

    QNetworkAccessManager nam;
    const QByteArray payload = QByteArray::fromStdString(request.body);
    QNetworkReply* reply = nullptr;
    switch (request.method) {
        case HttpMethod::Get:
            reply = nam.get(qreq);
            break;
        case HttpMethod::Post:
            reply = nam.post(qreq, payload);
            break;
        case HttpMethod::Put:
            reply = nam.put(qreq, payload);
            break;
        case HttpMethod::Delete:
            reply = nam.deleteResource(qreq);
            break;
    }
    if (reply == nullptr) {
        result.response.error = "Unsupported HTTP method";
        return result;
    }

    const int timeoutSec = request.timeoutSeconds > 0 ? request.timeoutSeconds : defaultTimeoutSeconds;
    const int timeoutMs = std::max(1000, timeoutSec * 1000);

    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);
    QObject::connect(&timer, &QTimer::timeout, reply, [reply]() {
        if (reply->isRunning()) {
            reply->abort();
        }
    });
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    timer.start(timeoutMs);
    loop.exec();

    const QVariant statusAttr = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
    result.response.statusCode = statusAttr.isValid() ? statusAttr.toInt() : 0;
    result.response.body = reply->readAll().toStdString();

    if (reply->error() != QNetworkReply::NoError) {
        const std::string err = reply->errorString().toStdString();
        if (result.response.error.empty()) {
            result.response.error = err;
        }
    }

    result.response.success = result.response.statusCode >= 200 && result.response.statusCode < 300;
    return result;
}

}  // namespace

HttpClient::HttpClient()
    : timeoutSeconds_(20)
{
}

HttpClient::HttpClient(int timeoutSeconds)
    : timeoutSeconds_(timeoutSeconds)
{
}

HttpClient::~HttpClient() = default;

void HttpClient::setTimeout(int seconds) {
    timeoutSeconds_ = seconds;
}

void HttpClient::setHeader(const std::string& key, const std::string& value) {
    defaultHeaders_[key] = value;
}

void HttpClient::clearHeaders() {
    defaultHeaders_.clear();
}

bool HttpClient::parseUrl(const std::string& url, std::string& host, std::string& port, std::string& path) {
    auto protocolEnd = url.find("://");
    if (protocolEnd == std::string::npos) {
        return false;
    }
    
    auto hostStart = protocolEnd + 3;
    auto pathStart = url.find('/', hostStart);
    
    host = url.substr(hostStart, pathStart - hostStart);
    
    auto portStart = host.find(':');
    if (portStart != std::string::npos) {
        port = host.substr(portStart + 1);
        host = host.substr(0, portStart);
    } else {
        port = "443";
    }
    
    if (pathStart == std::string::npos) {
        path = "/";
    } else {
        path = url.substr(pathStart);
    }
    
    return true;
}

std::string HttpClient::buildHttpRequest(const finsight::network::protocol::HttpRequest& request) {
    std::ostringstream stream;
    
    std::string methodStr;
    switch (request.method) {
        case finsight::network::protocol::HttpMethod::Get:
            methodStr = "GET";
            break;
        case finsight::network::protocol::HttpMethod::Post:
            methodStr = "POST";
            break;
        case finsight::network::protocol::HttpMethod::Put:
            methodStr = "PUT";
            break;
        case finsight::network::protocol::HttpMethod::Delete:
            methodStr = "DELETE";
            break;
    }
    
    std::string host, port, path;
    parseUrl(request.url, host, port, path);
    
    stream << methodStr << " " << path << " HTTP/1.1\r\n";
    stream << "Host: " << host << "\r\n";
    
    for (const auto& header : defaultHeaders_) {
        stream << header.first << ": " << header.second << "\r\n";
    }
    for (const auto& header : request.headers) {
        stream << header.first << ": " << header.second << "\r\n";
    }
    
    if (!request.body.empty()) {
        stream << "Content-Type: application/json\r\n";
        stream << "Content-Length: " << request.body.size() << "\r\n";
    }
    
    stream << "Connection: close\r\n";
    stream << "\r\n";
    
    if (!request.body.empty()) {
        stream << request.body;
    }
    
    return stream.str();
}

finsight::network::protocol::HttpResponse HttpClient::parseHttpResponse(const std::string& raw) {
    finsight::network::protocol::HttpResponse response;
    
    if (raw.empty()) {
        response.error = "Empty response";
        return response;
    }
    
    auto headerEnd = raw.find("\r\n\r\n");
    std::string headers;
    std::string body;
    
    if (headerEnd != std::string::npos) {
        headers = raw.substr(0, headerEnd);
        body = raw.substr(headerEnd + 4);
    } else {
        headers = raw;
    }
    
    std::istringstream headerStream(headers);
    std::string line;
    if (std::getline(headerStream, line)) {
        std::istringstream statusLine(line);
        std::string httpVersion;
        statusLine >> httpVersion >> response.statusCode >> response.statusMessage;
    }
    
    while (std::getline(headerStream, line)) {
        if (line.empty() || line == "\r") {
            break;
        }
        auto colonPos = line.find(':');
        if (colonPos != std::string::npos) {
            std::string key = line.substr(0, colonPos);
            std::string value = line.substr(colonPos + 1);
            if (!value.empty() && value[0] == ' ') {
                value = value.substr(1);
            }
            response.headers[key] = value;
        }
    }
    
    response.body = body;
    response.success = response.statusCode >= 200 && response.statusCode < 300;
    
    return response;
}

finsight::network::protocol::HttpResult HttpClient::sendRequest(const finsight::network::protocol::HttpRequest& request) {
    finsight::network::protocol::HttpResult result;
    result.attempted = true;

    if (request.url.size() >= 8 && request.url.compare(0, 8, "https://") == 0) {
        return sendHttpsViaQt(request, timeoutSeconds_, defaultHeaders_);
    }

    try {
        std::string host, port, path;
        if (!parseUrl(request.url, host, port, path)) {
            result.response.error = "Invalid URL";
            return result;
        }

        boost::asio::io_context io_context;
        boost::asio::ip::tcp::resolver resolver(io_context);
        auto endpoints = resolver.resolve(host, port);
        
        boost::asio::ip::tcp::socket socket(io_context);
        boost::asio::connect(socket, endpoints);
        
        std::string httpRequest = buildHttpRequest(request);
        boost::asio::write(socket, boost::asio::buffer(httpRequest));
        
        boost::asio::streambuf response;
        boost::system::error_code ec;
        boost::asio::read(socket, response, boost::asio::transfer_all(), ec);
        
        if (ec && ec != boost::asio::error::eof) {
            result.response.error = ec.message();
            return result;
        }
        
        std::string responseStr(boost::asio::buffer_cast<const char*>(response.data()));
        result.response = parseHttpResponse(responseStr);
        
    } catch (const std::exception& e) {
        result.response.error = e.what();
    } catch (...) {
        result.response.error = "Unknown error";
    }
    
    return result;
}

}  // namespace finsight::network::client