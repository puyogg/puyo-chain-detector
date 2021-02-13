#pragma once
#include <atomic>
#include <opencv2/core.hpp>
#include "app/threading.hpp"
#include "Settings.hpp"

namespace ChainDetector
{

void Detector(ThreadStatus& threadStatus, cv::Mat& qtPreview, CaptureSettings& captureSettings);

} // end namespace ChainDetector
