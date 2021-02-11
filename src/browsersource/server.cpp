#include "server.hpp"
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <functional>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

namespace websocket = boost::beast::websocket;
using tcp = boost::asio::ip::tcp;

void BrowserSource::sendChainData(ThreadStatus &threadStatus, tcp::socket socket)
{
    try
    {
        std::cout << "Launched a chain data thread." << std::endl;
        // Construct the stream by moving in the socket
        websocket::stream<tcp::socket> ws{std::move(socket)};

        // Set a decorator to change the Server of the handshake
        ws.set_option(websocket::stream_base::decorator(
            [](websocket::response_type &res) {
                res.set(boost::beast::http::field::server, "puyo-chain-detector");
            }));

        // Accept the websocket handshake
        ws.accept();

        // TODO: Instead of sending a message constantly, have the client in overlay.html make requests
        // at 60 FPS using requestAnimationFrame() loops.
        int x = 0;
        while (threadStatus.runWebsocket) {
            boost::beast::flat_buffer buffer;
            ws.read(buffer);
            std::string message = boost::beast::buffers_to_string(buffer.data());
            if (message == "puyo")
            {
                ws.write(boost::asio::buffer(std::to_string(x)));
            }
            x++;
        }

        return;
    }
    catch (boost::beast::system_error const &se)
    {
        // This indicates that the session was closed
        if (se.code() != websocket::error::closed)
            std::cerr << "Error (se): " << se.code().message() << std::endl;
        return;
    }
    catch (std::exception const &e)
    {
        std::cerr << "Error (e): " << e.what() << std::endl;
        return;
    }
}

void BrowserSource::waitForConnection(ThreadStatus &threadStatus)
{
    // Track a list of all opened socket threads
    threadStatus.websocketClosed = false;
    std::vector<std::thread> socketThreads;

    try
    {
        auto const address = boost::asio::ip::make_address("0.0.0.0");
        auto const port = static_cast<unsigned short>(2424);

        // Create io context
        boost::asio::io_context ioc{1};

        // Create an "acceptor" to handle incoming connections
        tcp::acceptor acceptor{ioc, {address, port}};

        // Use a blocking "infinite" loop to offer up multiple connections.
        while (threadStatus.runWebsocket)
        {
            // Create a socket that will receive a new connection
            tcp::socket socket{ioc};

            // Block until we get a connection
            acceptor.accept(socket);

            // Launch service in another thread and transfer ownership of the socket
            std::thread t = std::thread(&sendChainData, std::ref(threadStatus), std::move(socket));
            // Threads are move-only objects.
            socketThreads.push_back(std::move(t));
        }

        // Block until all the socket threads are joined.
        for (std::thread& th : socketThreads)
        {
            if (th.joinable()) th.join();
        }

        threadStatus.websocketClosed = true;
        std::cout << "Websocket Server closed." << std::endl;
        return;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error (connection setup): " << e.what() << std::endl;

        for (std::thread& th : socketThreads)
        {
            if (th.joinable()) th.join();
        }

        threadStatus.runWebsocket = false;
        threadStatus.websocketClosed = true;
        std::cout << "Websocket Server closed." << std::endl;
        return;
    }
}

