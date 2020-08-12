#pragma once
#include <opencv2/opencv.hpp>
#include <vector>
#include "PuyoField.h"
#include "CountField.h"
#include "StateController.h"

class GreenScreen
{
public:
	GreenScreen(bool debug = false);
	cv::Mat& screen();
	void update(std::vector<std::tuple<int64_t, int64_t, Color, int64_t>>& p0Data, std::vector<std::tuple<int64_t, int64_t, Color, int64_t>>& p1Data, ROIController& roiController);
private:
	bool m_debug;
	CountField m_countField;
	cv::Mat m_screen;
	cv::Mat m_cursorAlpha;
	cv::Mat m_numberAlpha;
	std::vector<cv::Mat> m_cursors;
	std::vector<cv::Mat> m_numbers;
	std::vector<cv::Rect> m_p0ROIs;
	std::vector<cv::Rect> m_p1ROIs;
	void resetScreen();
};

