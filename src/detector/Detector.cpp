#include <opencv2/core.hpp>
//#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include "Detector.hpp"
#include <iostream>
#include <string>
#include <thread>
#include "browsersource/server.hpp"

#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>


void ChainDetector::Detector(ThreadStatus& threadStatus, cv::Mat& qtPreview, int deviceID, int modeID)
{
    // Toggle thread status
    threadStatus.detectorClosed = false;

    cv::VideoCapture cap;
    cap.open(deviceID, modeID);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, 270);
    cap.set(cv::CAP_PROP_FRAME_WIDTH, 480);

    if (!cap.isOpened())
    {
        std::cerr << "Couldn't open the capture feed.\n";
        return;
    }

    // Initialize data structures to share with the websocket service

    // Spawn a websocket service in another thread
    std::thread websocketThread = std::thread(&BrowserSource::waitForConnection, std::ref(threadStatus));

    // Initialize the mats the analysis will need
    cv::Mat frame;

    // Detection loop
    while (threadStatus.runDetector)
    {
        // Receive current frame from capture device and resize it.
        cap >> frame;
        cv::resize(frame, frame, cv::Size(480, 270), 0, 0, cv::INTER_LINEAR);
        frame.copyTo(qtPreview);

        if (frame.empty())
        {
            std::cout << "Input data stream was empty. Aborting.\n";
            threadStatus.runDetector = false;
            break;
        }

        if (threadStatus.runDetector == false || threadStatus.runWebsocket == false)
        {
            break;
        }
    }
    // Don't need to run websockets anymore if detection loop is finished.
    threadStatus.runWebsocket = false;

    // Make a dummy connection to the websocket server just to get the acceptor to unblock lol.
    // I don't feel like figuring out async right now...
    // https://github1s.com/boostorg/beast/blob/HEAD/example/websocket/client/sync/websocket_client_sync.cpp
    std::string host = "127.0.0.1";
    std::string port = "2424";
    boost::asio::io_context ioc;
    tcp::resolver resolver{ ioc };
    websocket::stream<tcp::socket> ws{ ioc };
    auto const results = resolver.resolve(host, port);
    auto ep = boost::asio::connect(ws.next_layer(), results);
    host += ':' + std::to_string(ep.port());
    ws.set_option(websocket::stream_base::decorator(
        [](websocket::request_type& req)
        {
            req.set(http::field::user_agent,
                std::string(BOOST_BEAST_VERSION_STRING) +
                    " puyo-chain-detector");
        }));
    ws.handshake(host, "/");

    // Block until the websocket server is closed.
    websocketThread.join();

    std::cout << "Chain Detector closed." << std::endl;
    threadStatus.detectorClosed = true;
    return;
}
