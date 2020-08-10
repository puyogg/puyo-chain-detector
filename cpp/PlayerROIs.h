#pragma once
#include <opencv2/opencv.hpp>
#include <deque>
#include "FieldTracker.h"
#include "NextTracker.h"
#include "ScoreXTracker.h"

class PlayerROIs
{
public:
	PlayerROIs(int player);
	void update(std::vector<cv::Rect>& fieldROI, cv::Mat& roiAnalysis, cv::Mat& nextAnalysis, cv::Mat& scoreAnalysis);
	cv::Rect medianField();
	int fieldMaxY();
	void drawROIs(cv::Mat& frame);

	// getters
	FieldTracker& field();
	NextTracker& nextWindow();
	ScoreXTracker& scoreX();
private:
	int m_player;
	FieldTracker m_field;
	NextTracker m_next;
	ScoreXTracker m_score;
};

