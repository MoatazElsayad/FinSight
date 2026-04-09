#pragma once

#include <boost/asio.hpp>
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

    boost::asio::io_context io_context_;
    boost::asio::ip::tcp::socket socket_;
};
