#pragma once
#include <opencv2/opencv.hpp>
#include <vector>
#include "Constants.h"
class ScoreXTracker
{
public:
	ScoreXTracker();
	ScoreXTracker(int player);
	void update(cv::Rect& medianField, cv::Mat& scoreAnalysis);
	cv::Rect& getROI();
	bool& hasX();
private:
	int m_player;
	bool m_hasX;
	cv::Rect m_scoreROI;
	cv::Rect scoreROIFromField(cv::Rect& medianField);
};

