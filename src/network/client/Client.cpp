#include "network/client/Client.h"

#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

Client::Client(const std::string& host, int port)
    : host_(host), port_(port), socket_fd_(-1) {}

bool Client::connectToServer() {
    socket_fd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd_ < 0) {
        return false;
    }

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port_);

    if (inet_pton(AF_INET, host_.c_str(), &server_addr.sin_addr) <= 0) {
        return false;
    }

    if (connect(socket_fd_, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr)) < 0) {
        return false;
    }

    return true;
}

bool Client::sendMessage(const std::string& message) {
    if (socket_fd_ < 0) {
        return false;
    }

    ssize_t sent = send(socket_fd_, message.c_str(), message.size(), 0);
    return sent == static_cast<ssize_t>(message.size());
}

std::string Client::receiveMessage() {
    if (socket_fd_ < 0) {
        return "";
    }

    char buffer[1024] = {0};
    ssize_t received = recv(socket_fd_, buffer, sizeof(buffer) - 1, 0);

    if (received <= 0) {
        return "";
    }

    return std::string(buffer, received);
}

void Client::closeConnection() {
    if (socket_fd_ >= 0) {
        close(socket_fd_);
        socket_fd_ = -1;
    }
}
