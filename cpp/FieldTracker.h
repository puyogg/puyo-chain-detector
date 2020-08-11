#pragma once
#include <opencv2/opencv.hpp>
#include <iostream>
#include <algorithm>
#include <cmath>
#include <deque>
#include "Constants.h"

class FieldTracker
{
public:
	FieldTracker(size_t historySize = 301);
	FieldTracker(int player, size_t historySize = 301);
	void update(std::vector<cv::Rect>& fieldROI);
	cv::Rect& getROI();
	cv::Rect getMedian();
	int getMaxY();
private:
	cv::Rect m_activeROI;
	cv::Rect m_default;
	std::deque<cv::Rect> m_history;
	size_t m_historySize;
};