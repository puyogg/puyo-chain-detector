#include <opencv2/core.hpp>
#include "Settings.hpp"

namespace ChainDetector {

class ScoreTracker
{
private:
    int m_player;
    CaptureSettings* m_settings;

public:
    ScoreTracker(int player, CaptureSettings& captureSettings);
    bool isPopping(cv::Mat& frame);
};

} // end namespace ChainDetector
