#pragma once
#include <opencv2/core.hpp>
#include <opencv2/dnn.hpp>
#include "Settings.hpp"
#include "chainsim/Fields.hpp"
#include "NextTracker.hpp"
#include "ScoreTracker.hpp"
#include <vector>
#include <tuple>
#include <string>

namespace ChainDetector {

class PlayerTracker
{
private:
    int m_player;
    cv::dnn::Net* m_net;
    CaptureSettings* m_settings;
    std::vector<cv::Rect> m_cellRects{ 72 };
    NextTracker m_nextTracker;
    ScoreTracker m_scoreTracker;
public:
    PlayerTracker(int player, cv::dnn::Net& net, CaptureSettings& captureSettings);
    void update(cv::Mat& frame);
    std::vector<Chainsim::Color> readFieldColors(cv::Mat& fieldMat);
    Chainsim::IntField m_lengthField;
    Chainsim::PuyoField m_popColors;
    Chainsim::PuyoField m_puyoField;
};

} // end Chain Detector Namespace
