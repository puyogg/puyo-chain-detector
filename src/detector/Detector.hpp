#pragma once
#include <atomic>

namespace ChainDetector
{

void Detector(std::atomic<bool>& running, int deviceID, int modeID);

} // end namespace ChainDetector
