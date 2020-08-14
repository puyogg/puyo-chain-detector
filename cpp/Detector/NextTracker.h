#pragma once
#include <vector>
#include <opencv2/opencv.hpp>
class NextTracker
{
public:
	NextTracker();
	NextTracker(int player);
	void update(cv::Rect& medianField, cv::Mat& nextAnalysis);
	std::vector<cv::Rect> getROIs();
	bool& isMoving();
private:
	int m_player;
	cv::Mat m_prevTop;
	cv::Mat m_prevBot;
	cv::Mat m_top;
	cv::Mat m_bot;
	cv::Rect m_topRoi;
	cv::Rect m_botRoi;
	bool m_isMoving;
	cv::Rect nextROIFromField(cv::Rect& medianField);
	void setTopBotROIs(cv::Rect& nextROI);
};

