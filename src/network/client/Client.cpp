#include "network/client/Client.h"

#include <boost/asio/connect.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/write.hpp>

Client::Client(const std::string& host, int port)
    : host_(host),
      port_(port),
      socket_(io_context_) {}

bool Client::connectToServer() {
    try {
        boost::asio::ip::tcp::resolver resolver(io_context_);
        auto endpoints = resolver.resolve(host_, std::to_string(port_));
        boost::asio::connect(socket_, endpoints);
        return true;
    } catch (...) {
        return false;
    }
}

bool Client::sendMessage(const std::string& message) {
    try {
        if (!socket_.is_open()) {
            return false;
        }

        boost::asio::write(socket_, boost::asio::buffer(message));
        return true;
    } catch (...) {
        return false;
    }
}

std::string Client::receiveMessage() {
    try {
        if (!socket_.is_open()) {
            return "";
        }

        char buffer[1024];
        std::size_t len = socket_.read_some(boost::asio::buffer(buffer));
        return std::string(buffer, len);
    } catch (...) {
        return "";
    }
}

void Client::closeConnection() {
    if (socket_.is_open()) {
        boost::system::error_code ec;
        socket_.close(ec);
    }
}
