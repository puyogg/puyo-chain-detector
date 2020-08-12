#include "PlayerROIs.h"

PlayerROIs::PlayerROIs(int player)
{
	m_player = player;
	m_field = FieldTracker(player);
	m_next = NextTracker(player);
	m_score = ScoreXTracker(player);
}

void PlayerROIs::update(std::vector<cv::Rect>& fieldROI, cv::Mat& roiAnalysis, cv::Mat& nextAnalysis, cv::Mat& scoreAnalysis)
{
	// Update the field
	m_field.update(fieldROI);

	// Use the median field positions to decide on the positions of
	// the Next Window and Score Region
	cv::Rect medianField{ m_field.getMedian() };
	m_next.update(medianField, nextAnalysis);

	m_score.update(medianField, scoreAnalysis);
}

void PlayerROIs::drawROIs(cv::Mat& frame)
{
	cv::Scalar color = (m_player == 0) ? cv::Scalar(0, 0, 255) : cv::Scalar(255, 0, 0);

	cv::rectangle(frame, m_field.getROI().tl(), m_field.getROI().br(), color, 2);

	auto windowROIs{ m_next.getROIs() };
	cv::rectangle(frame, windowROIs[0].tl(), windowROIs[0].br(), color, 2);
	cv::rectangle(frame, windowROIs[1].tl(), windowROIs[1].br(), color, 2);

	cv::rectangle(frame, m_score.getROI().tl(), m_score.getROI().br(), color, 2);
}


cv::Rect PlayerROIs::medianField()
{
	return m_field.getMedian();
}

int PlayerROIs::fieldMaxY()
{
	return m_field.getMaxY();
}

FieldTracker& PlayerROIs::field()
{
	return m_field;
}

NextTracker& PlayerROIs::nextWindow()
{
	return m_next;
}

ScoreXTracker& PlayerROIs::scoreX()
{
	return m_score;
}