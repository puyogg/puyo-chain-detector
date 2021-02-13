#include "ScoreTracker.hpp"
#include <opencv2/imgproc.hpp>
#include <iostream>

namespace ChainDetector {

ScoreTracker::ScoreTracker(int player, CaptureSettings& captureSettings)
{
    m_player = player;
    m_settings = &captureSettings;
}

bool ScoreTracker::isPopping(cv::Mat& frame)
{
    cv::Rect& bonusRect = m_settings->player.at(m_player).bonusRect;
    double fractionThreshold = m_settings->bonusThreshold;

    // Get a crop of the bonus region
    cv::Mat bonusMat = frame(bonusRect);
    cv::cvtColor(bonusMat, bonusMat, cv::COLOR_BGR2GRAY);

    // Apply the researched binary threshold.
    cv::threshold(bonusMat, bonusMat, 180, 1, cv::THRESH_BINARY);

    // Get the sum of white pixels
    int whiteSum = cv::sum(bonusMat)[0];
    // Get the total amount of pixels
    int total = bonusMat.cols * bonusMat.rows;
    // Get the black fraction
    double blackFraction = 1.0 - static_cast<double>(whiteSum) / total;

//    // Preview thresholding
//    cv::cvtColor(bonusMat, bonusMat, cv::COLOR_GRAY2BGR);
//    bonusMat.copyTo(frame(bonusRect));

    return blackFraction >= fractionThreshold;
}

} // end namespace ChainDetector
