#pragma once
#include "IMessageHandler.h"
#include <boost/asio.hpp>
#include <memory>
#include <string>

namespace finsight::network::tcp {

using boost::asio::ip::tcp;

class Session : public std::enable_shared_from_this<Session> {
public:
    Session(tcp::socket socket, IMessageHandler& handler);
    void start();

private:
    void readLine();
    void writeLine(std::string message);

    tcp::socket           socket_;
    boost::asio::streambuf buffer_;
    IMessageHandler&      handler_;
};

class TcpServer {
public:
    TcpServer(boost::asio::io_context& ioc,
              unsigned short port,
              IMessageHandler& handler);

private:
    void doAccept();

    tcp::acceptor    acceptor_;
    IMessageHandler& handler_;
};

} 
