#include "Constants.hpp"
#include "NextTracker.hpp"
#include <opencv2/imgproc.hpp>
#include <vector>
#include <array>
#include <algorithm>
#include <utility>
#include <iostream>

namespace ChainDetector {

NextTracker::NextTracker(int player, CaptureSettings& captureSettings)
{
    m_isMoving = false;
    m_player = player;
    m_settings = &captureSettings;
    m_cooldown = 0;
    m_maxCooldown = 12;
}

bool NextTracker::isMoving(cv::Mat& frame)
{
    // Use captureSettings nextRects to get
    // adaptively thresholded versions of each Next Puyo
    std::array<cv::Rect, 4>& nextRects = m_settings->player.at(m_player).nextRects;

    std::array<cv::Mat, 4> nextMats;
    for (int i = 0; i < 4; i++)
    {
        cv::Mat nextMat = frame(nextRects[i]);
        cv::cvtColor(nextMat, nextMat, cv::COLOR_BGR2GRAY);
        cv::adaptiveThreshold(nextMat, nextMat, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C, cv::THRESH_BINARY, 3, 3);
        nextMats.at(i) = std::move(nextMat);
    }

    // Empty case (the app just started, so there's no prevs to analyze
    if (m_prevs.at(0).empty() || m_prevs.at(1).empty() || m_prevs.at(2).empty() || m_prevs.at(3).empty())
    {
        for (int i = 0; i < 4; i++)
        {
            nextMats.at(i).copyTo(m_prevs.at(i));
        }
    }

    // Don't need to recalculate if the next window movement was noticed recently
    if (m_cooldown > 0)
    {
        m_cooldown--;
        return false;
    }

    // Subtract nextMats from their previous frames
    int passCount = 0;
    const double area = static_cast<double>(PUYO_W * PUYO_H);
    for (int i = 0; i < 4; i++)
    {
        cv::Mat diff;
        cv::absdiff(nextMats.at(i), m_prevs.at(i), diff);
        diff.convertTo(diff, CV_16UC1, 1.0 / 255);
        int sumDiffPixels = static_cast<int>(cv::sum(diff)[0]);
        bool pass = sumDiffPixels > (area * 0.1);
        if (pass) passCount++;
    }

    if (passCount >= 3)
    {
        m_cooldown = m_maxCooldown;
        for (int i = 0; i < 4; i++)
        {
            nextMats.at(i).copyTo(m_prevs.at(i));
        }
        return true;
    }

    return false;
}

} // end namespace ChainDetector
