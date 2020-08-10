#pragma once
#include <opencv2/opencv.hpp>
#include <vector>
#include <cmath> // std::abs
#include "PlayerROIs.h"
#include "Constants.h"

class ROIController
{
public:
	ROIController();
	void update(cv::Mat& frame, cv::Mat& roiAnalysis, cv::Mat& nextAnalysis, cv::Mat& scoreAnalysis);
	PlayerROIs& player(int id);

private:
	PlayerROIs m_player0{ 0 };
	PlayerROIs m_player1{ 1 };
	std::vector<cv::Rect> findFields(cv::Mat& frame, cv::Mat& roiAnalysis);
};

