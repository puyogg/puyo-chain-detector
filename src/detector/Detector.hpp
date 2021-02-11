#pragma once
#include <atomic>
#include <opencv2/core.hpp>
#include "app/threading.hpp"

namespace ChainDetector
{

void Detector(ThreadStatus& threadStatus, cv::Mat& qtPreview, int deviceID, int modeID);

} // end namespace ChainDetector
