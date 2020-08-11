#pragma once
#include <opencv2/opencv.hpp>
#include <vector>
#include "ROIController.h"
#include "Constants.h"
#include "PuyoField.h"

class StateController
{
public:
	StateController(int player, cv::dnn::Net& net, bool tryIgnorePopping);
	void update(cv::Mat& frame, ROIController& roiController, bool reset);
	std::vector<int64_t> field();
	std::vector<std::tuple<int64_t, int64_t, Color, int64_t>>& cursorData();
	std::vector<cv::Mat> m_cellMats{ 72 };
private:
	int m_player;
	bool m_tryIgnorePopping;
	std::vector<std::tuple<int64_t, int64_t, Color, int64_t>> m_cursorData;
	cv::Mat m_centerCrop;
	cv::dnn::Net m_net;
	std::vector<int64_t> m_field;
	std::vector<cv::Rect> m_cellROIs{ 72 };
	std::vector<std::vector<int64_t>> m_tryInds;
	std::vector<std::vector<int64_t>> m_tryColors;
	std::vector<int64_t> chainLengths;
	//std::vector<cv::Mat> m_cellMats{ 72 };
	void setCellMats();
	void predictField();
};

