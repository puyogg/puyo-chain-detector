#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include "Detector.hpp"
#include <iostream>
#include <string>

void ChainDetector::Detector(int deviceID, int modeID)
{
    std::string windowName{ "Puyo Chain Detector" };
    cv::namedWindow(windowName, cv::WINDOW_AUTOSIZE);
    cv::VideoCapture cap;

    cap.open(deviceID, modeID);

    // Request the capture card to send its feed as 960x540
    // Some cards aren't able to set certain custom resolutions though.
    // The image may have to be resized manually.
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, 540);
    cap.set(cv::CAP_PROP_FRAME_WIDTH, 960);

    if (!cap.isOpened())
    {
        std::cerr << "Couldn't open the capture feed.\n";
        return;
    }

    // Initialize the mats the analysis will need
    cv::Mat input;

    while (true)
    {
        cap >> input;
        if (input.empty())
        {
            std::cout << "Input data stream was empty. Aborting.\n";
            break;
        }

        cv::imshow(windowName, input);

        char c = cv::waitKey(1);
        if (c == 27)
        {
            break;
        }
        else if (cv::getWindowProperty(windowName, cv::WND_PROP_VISIBLE) < 1)
        {
            break;
        }
    }
}
