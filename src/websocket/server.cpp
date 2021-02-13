#include "server.hpp"
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <functional>
#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <json/json.h>

namespace websocket = boost::beast::websocket;
using tcp = boost::asio::ip::tcp;

void BrowserSource::sendChainData(ThreadStatus& threadStatus, std::array<DataFields, 2>& playerData, tcp::socket socket)
{
    try
    {
        std::cout << "Launched a chain data thread." << std::endl;

        // Allocate a Json::StreamWriterBuilder for later
        Json::StreamWriterBuilder builder;
        builder["indentation"] = "";

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
        while (threadStatus.runWebsocket) {
            boost::beast::flat_buffer buffer;
            ws.read(buffer);
            std::string message = boost::beast::buffers_to_string(buffer.data());
            if (message == "puyo")
            {
                Json::Value cursorData(Json::arrayValue);

                int arrayLength = playerData.at(0).colorField->m_data.size();
                for (int i = 0; i < 2; i++)
                {
                    Json::Value p;
                    Json::Value length(Json::arrayValue);
                    Json::Value colors(Json::arrayValue);
                    for (int j = 0; j < arrayLength; j++)
                    {
                        length.append(playerData.at(i).lengthField->m_data.at(j));
                        colors.append(static_cast<int>(playerData.at(i).colorField->m_data.at(j)));
                    }
                    p["lengths"] = length;
                    p["colors"] = colors;
                    cursorData.append(p);
                }

                std::string output = Json::writeString(builder, cursorData);

                ws.write(boost::asio::buffer(output));
            }
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

void BrowserSource::waitForConnection(ThreadStatus& threadStatus, std::array<DataFields, 2>& playerData)
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
            std::thread t = std::thread(&sendChainData, std::ref(threadStatus), std::ref(playerData), std::move(socket));
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

