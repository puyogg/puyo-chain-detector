#pragma once
#include <boost/beast/core.hpp>
#include <boost/asio/ip/tcp.hpp>
#include "app/threading.hpp"

namespace BrowserSource {
namespace websocket = boost::beast::websocket;
using tcp = boost::asio::ip::tcp;
void sendChainData(ThreadStatus &threadStatus, tcp::socket socket);
void waitForConnection(ThreadStatus &threadStatus);

} // end namespace BrowserSource
