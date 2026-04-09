#pragma once

#include <string>

class Client {
public:
    Client(const std::string& host, int port);

    bool connectToServer();
    bool sendMessage(const std::string& message);
    std::string receiveMessage();
    void closeConnection();

private:
    std::string host_;
    int port_;
    int socket_fd_;
};

