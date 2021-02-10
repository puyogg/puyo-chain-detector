#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <functional>
#include <iostream>
#include <string>
#include <thread>

namespace websocket = boost::beast::websocket;
using tcp = boost::asio::ip::tcp;

void sendChainData(tcp::socket socket)
{
    try
    {
        std::cout << "Launched a chain data thread." << std::endl;
        // Construct the stream by moving in the socket
        websocket::stream<tcp::socket> ws{ std::move(socket) };

        // Set a decorator to change the Server of the handshake
        ws.set_option(websocket::stream_base::decorator(
            [](websocket::response_type& res)
            {
                res.set(boost::beast::http::field::server, "chain-data");
            }
            ));

        // Accept the websocket handshake
        ws.accept();

        // TODO: Instead of sending a message constantly, have the client in overlay.html make requests
        // at 60 FPS using requestAnimationFrame() loops.
        int x = 0;
        for(;;)
        {
            // Constantly send a message
            ws.write(boost::asio::buffer(std::to_string(x)));
            x++;
        }
    }
    catch(boost::beast::system_error const& se)
    {
        // This indicates that the session was closed
        if (se.code() != websocket::error::closed)
            std::cerr << "Error: " << se.code().message() << std::endl;
    }
    catch(std::exception const& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}

int main()
{
    try
    {
        auto const address = boost::asio::ip::make_address("0.0.0.0");
        auto const port = static_cast<unsigned short>(2424);

        // Create io context
        boost::asio::io_context ioc{1};

        // Create an "acceptor" to handle incoming connections
        tcp::acceptor acceptor{ ioc, { address, port }};

        // Use a blocking "infinite" loop to offer up multiple connections.
        for(;;)
        {
            // Create a socket that will receive a new connection
            tcp::socket socket{ ioc };

            // Block until we get a connection
            acceptor.accept(socket);

            // Launch service in another thread and transfer ownership of the socket
            std::thread(&sendChainData, std::move(socket)).detach();
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
}
