#include "network/tcp/TcpServer.h"
#include <iostream>

namespace finsight::network::tcp {
Session::Session(tcp::socket socket, IMessageHandler& handler)
    : socket_(std::move(socket)), handler_(handler) {}
void Session::start() { readLine(); }
void Session::readLine() {
    auto self = shared_from_this();
    boost::asio::async_read_until(socket_, buffer_, '\n',
        [this, self](boost::system::error_code ec, std::size_t) {
            if (ec) {
                if (ec != boost::asio::error::eof)
                    std::cerr << "[server] Read error: " << ec.message() << "\n";
                return;
            }
            std::istream stream(&buffer_);
            std::string line;
            std::getline(stream, line);
            if (!line.empty() && line.back() == '\r') line.pop_back();

            std::string response;
            try {
                response = handler_.handle(line);
            } catch (const std::exception& e) {
                response = std::string("{\"status\":\"error\",\"message\":\"")
                           + e.what() + "\"}";
            }
            writeLine(response + "\n");
        });
}

void Session::writeLine(std::string message) {
    auto self = shared_from_this();
    boost::asio::async_write(socket_, boost::asio::buffer(message),
        [this, self](boost::system::error_code ec, std::size_t) {
            if (!ec) readLine();
        });
}

TcpServer::TcpServer(boost::asio::io_context& ioc,
                     unsigned short port,
                     IMessageHandler& handler)
    : acceptor_(ioc, tcp::endpoint(tcp::v4(), port))
    , handler_(handler)
{
    doAccept();
}

void TcpServer::doAccept() {
    acceptor_.async_accept(
        [this](boost::system::error_code ec, tcp::socket socket) {
            if (!ec) {
                std::cout << "[server] Client connected.\n";
                std::make_shared<Session>(std::move(socket), handler_)->start();
            }
            doAccept();
        });
}

} 
