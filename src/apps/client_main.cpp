#include <boost/asio.hpp>
#include <iostream>
#include <string>

using boost::asio::ip::tcp;

int main(int argc, char* argv[]) {
    const std::string host = (argc > 1) ? argv[1] : "127.0.0.1";
    const std::string port = (argc > 2) ? argv[2] : "9090";
try {
        boost::asio::io_context ioc;
        tcp::resolver resolver(ioc);
        tcp::socket   socket(ioc);
        boost::asio::connect(socket, resolver.resolve(host, port));
 std::cout << "Connected. Type JSON commands, Ctrl+D to quit.\n";
 std::cout << "Example: {\"command\":\"ping\"}\n\n";

 std::string line;
	 while (std::getline(std::cin, line)) {
            if (line.empty()) continue;
            boost::asio::write(socket, boost::asio::buffer(line + "\n"));

            boost::asio::streambuf buf;
            boost::asio::read_until(socket, buf, '\n');
            std::istream stream(&buf);
            std::string response;
            std::getline(stream, response);
            std::cout << "Response: " << response << "\n";
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
}
