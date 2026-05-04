#include <boost/asio.hpp>
#include <iostream>
#include "network/tcp/TcpServer.h"
#include "network/tcp/BackendMessageHandler.h"

int main() {
 const unsigned short port = 9090;
 finsight::core::managers::FinanceTrackerBackend backend;
 BackendMessageHandler handler(backend);

 boost::asio::io_context ioc;
 finsight::network::tcp::TcpServer server(ioc, port, handler);

 std::cout << "FinSight server listening on port " << port << "\n";
 std::cout << "Commands: ping, register, login, "
                 "list_transactions, dashboard_summary\n";
 std::cout << "Ctrl+C to stop.\n";
 ioc.run();
}
