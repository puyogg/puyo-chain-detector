#include <array>
#include <opencv2/core.hpp>
#include "Settings.hpp"

namespace ChainDetector {

class NextTracker
{
private:
    std::array<cv::Mat, 4> m_prevs;
    bool m_isMoving;
    int m_player;
    CaptureSettings* m_settings;
    int m_cooldown;
    int m_maxCooldown;
public:
    NextTracker(int player, CaptureSettings& captureSettings);
    bool isMoving(cv::Mat& frame);
};

} // end namespace ChainDetector
