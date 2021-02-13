#include <opencv2/core.hpp>
//#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/dnn.hpp>
#include <iostream>
#include <string>
#include <thread>
#include <array>

#include "Detector.hpp"
#include "Settings.hpp"
#include "PlayerTracker.hpp"

#include "websocket/server.hpp"

#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>


void ChainDetector::Detector(ThreadStatus& threadStatus, cv::Mat& qtPreview, CaptureSettings& captureSettings)
{
    // Toggle thread status
    threadStatus.detectorClosed = false;

    cv::VideoCapture cap;
    cap.open(captureSettings.deviceID, captureSettings.mode);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, 270);
    cap.set(cv::CAP_PROP_FRAME_WIDTH, 480);

    if (!cap.isOpened())
    {
        std::cerr << "Couldn't open the capture feed.\n";
        threadStatus.captureFailed = true;
        threadStatus.detectorClosed = true;
        threadStatus.runDetector = false;
        return;
    }

    // Load MLP
    cv::dnn::Net net = cv::dnn::readNetFromONNX("puyo-mlp-gpu.onnx");

    // Set up player data. The Chain Detector writes to it; the websocket threads read from it
    PlayerTracker p0{ 0, net, captureSettings };
    PlayerTracker p1{ 1, net, captureSettings };
    std::array<BrowserSource::DataFields, 2> playerData;
    playerData.at(0) = BrowserSource::DataFields{ &(p0.m_lengthField), &(p0.m_colorField) };
    playerData.at(1) = BrowserSource::DataFields{ &(p1.m_lengthField), &(p1.m_colorField) };

    // Spawn a websocket service in another thread
    std::thread websocketThread = std::thread(&BrowserSource::waitForConnection, std::ref(threadStatus), std::ref(playerData) );

    // Initialize the mats the analysis will need
    cv::Mat frame;

    // Detection loop
    std::cout << "Beginning Detection loop...." << std::endl;
    while (threadStatus.runDetector)
    {
        // Receive current frame from capture device and resize it.
        cap >> frame;
        cv::resize(frame, frame, cv::Size(480, 270), 0, 0, cv::INTER_LINEAR);

        p0.update(frame);
        p1.update(frame);

        // Preview to show in the Qt App
        if (captureSettings.enablePreview)
        {
            frame.copyTo(qtPreview);
        }

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
