#pragma once
#include <boost/beast/core.hpp>
#include <boost/asio/ip/tcp.hpp>
#include "app/threading.hpp"
#include "chainsim/Fields.hpp"
#include <array>
#include <string>

namespace BrowserSource {

struct DataFields
{
    Chainsim::IntField* lengthField;
    Chainsim::PuyoField* colorField;
};

namespace websocket = boost::beast::websocket;
using tcp = boost::asio::ip::tcp;
void sendChainData(ThreadStatus& threadStatus, std::array<DataFields, 2>& playerData, tcp::socket socket);
void waitForConnection(ThreadStatus& threadStatus, std::array<DataFields, 2>& playerData);

} // end namespace BrowserSource
