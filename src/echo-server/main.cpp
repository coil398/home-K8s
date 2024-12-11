#include <iostream>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

int main() {
    try {
        boost::asio::io_context io_context;

        tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), 8080));

        for (;;) {
            tcp::socket socket(io_context);
            acceptor.accept(socket);

            std::array<char, 128> data;
            boost::system::error_code error;

            size_t length = socket.read_some(boost::asio::buffer(data), error);

            if (error == boost::asio::error::eof)
                break; // Connection closed cleanly by peer.
            else if (error)
                throw boost::system::system_error(error); // Some other error.

            boost::asio::write(socket, boost::asio::buffer(data, length));
        }
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}
