// Detector.cpp : This file contains the 'main' function. Program execution begins and ends there.

#include <iostream>
#include <string>
#include <opencv2/opencv.hpp>
#include <fstream>
#include <json/json.h>
#include "ROIController.h"
#include "FPS.h"
#include "Constants.h"
#include "StateController.h"
#include "GreenScreen.h"

int main()
{
    // Load settings from JSON file.
    std::ifstream settingsFile("settings.json", std::ifstream::binary);
    Json::Value settings;
    settingsFile >> settings;

    // Set up capture window
    std::string windowName{ "Puyo Chain Detector" };
    cv::namedWindow(windowName, cv::WINDOW_AUTOSIZE);
    cv::VideoCapture cap;
    //cap.open(0, cv::CAP_DSHOW);
    cap.open(settings["device_id"].asInt(), settings["mode"].asInt());

    // Request the capture card to send its feed as 960x540.
    // Some cards aren't able to do this though. So the image may have to be resized later on.
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, VIDEO_HEIGHT);
    cap.set(cv::CAP_PROP_FRAME_WIDTH, VIDEO_WIDTH);

    if (!cap.isOpened())
    {
        std::cerr << "Couldn't open capture.\n";
        return -1;
    }

    // Declare the mats the analysis will need.
    cv::Mat input;
    cv::Mat frame;
    cv::Mat gray;
    cv::Mat roiAnalysis;
    cv::Mat nextAnalysis;
    cv::Mat scoreAnalysis;

    // Grab a test frame. Is everything the right size?
    cap >> frame;
    bool resizeFrame{ false };

    if (frame.cols != static_cast<int>(VIDEO_WIDTH) || frame.rows != static_cast<int>(VIDEO_HEIGHT))
    {
        std::cout << "Capture card wasn't able to provide 960x540 video. Turned on manual resizing.\n";
        resizeFrame = true;
    }

    // ROI Controller
    ROIController roiController;

    // Load neural network
    cv::dnn::Net net = cv::dnn::readNetFromONNX("puyo-mlp-gpu.onnx");

    // Player States
    //bool tryIgnorePopping{ settings["try_ignore_popping"].asBool() }; // Try to show less glitchy outputs
    bool tryIgnorePopping{ true };
    StateController player0{ 0, net, tryIgnorePopping };
    StateController player1{ 1, net, tryIgnorePopping };

    // Green Screen
    GreenScreen greenScreen;

    // FPS Tracker
    FPS fps;

    // Show the captured feed to validate the feed
    for (int i = 0; i < 120; i++)
    {
        cap >> input;
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

    while (true)
    {
        // Get video stream
        cap >> input;
        if (input.empty())
        {
            std::cout << "Input data stream was empty. Aborting.\n";
            break;
        }

        if (resizeFrame)
        {
            cv::resize(input, frame, cv::Size(static_cast<int>(VIDEO_WIDTH), static_cast<int>(VIDEO_HEIGHT)), 0, 0, cv::INTER_LINEAR);
        }
        else
        {
            frame = input;
        }

        cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
        gray.copyTo(roiAnalysis);
        gray.copyTo(nextAnalysis);
        gray.copyTo(scoreAnalysis);

        // Send frames to ROIController to update all the ROIs
        roiController.update(frame, roiAnalysis, nextAnalysis, scoreAnalysis);

        // Check if the screen turned to black
        cv::Scalar mean = cv::mean(gray);
        bool reset = mean.val[0] > 210 || mean.val[0] < 20;

        //// Send ROIs to StateControllers so they can predict the chains
        player0.update(frame, roiController, reset);
        player1.update(frame, roiController, reset);

        //cv::Mat p0field = frame(roiController.player(0).field().getROI());
        //cv::imshow(windowName, greenScreen.screen());
        greenScreen.update(player0.cursorData(), player1.cursorData());
        cv::imshow(windowName, greenScreen.screen());

        // Display Average FPS for each second
        fps.tick();

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

    cap.release();
    return 0;
}