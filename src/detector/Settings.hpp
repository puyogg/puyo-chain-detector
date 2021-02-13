#pragma once
#include <opencv2/core.hpp>
#include <vector>
#include <array>
#include <string>
#include <atomic>

namespace ChainDetector
{

struct PlayerRects
{
    cv::Rect fieldRect;
    cv::Rect scoreRect;
    cv::Rect bonusRect;
    std::array<cv::Rect, 4> nextRects;
};

struct CaptureSettings
{
    int deviceID;
    int mode;
    std::string preset;
    double bonusThreshold;
    bool enablePreview;
    std::array<PlayerRects, 2> player;
};

}
