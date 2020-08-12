#include "NextTracker.h"

NextTracker::NextTracker()
{
	m_player = 0;
	m_isMoving = false;
}

NextTracker::NextTracker(int player)
{
	m_player = player;
	m_isMoving = false;
}

void NextTracker::update(cv::Rect& medianField, cv::Mat& nextAnalysis)
{
	// Get the bounding box for the whole next window
	cv::Rect nextROI{ nextROIFromField(medianField) };
	// Isolate the small parts of the next window to detect movement in
	setTopBotROIs(nextROI);

	// Apply adaptive threshold to the subregion of nextAnalysis
	cv::Mat nextWindow = nextAnalysis(nextROI);
	cv::adaptiveThreshold(nextWindow, nextWindow, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C, cv::THRESH_BINARY, 5, 5);

	// Crops
	m_top = nextAnalysis(m_topRoi);
	m_bot = nextAnalysis(m_botRoi);

	// Empty exception
	if (m_prevTop.empty() || m_prevBot.empty())
	{
		std::cout << "Not going to do anything because prevs are empty.\n";
		m_top.copyTo(m_prevTop);
		m_bot.copyTo(m_prevBot);
		return;
	}

	// Skip computing the frame difference if the sizes don't match.
	int topRows = m_top.rows;
	int topCols = m_top.cols;
	int prevTopRows = m_prevTop.rows;
	int prevTopCols = m_prevTop.cols;
	int botRows = m_bot.rows;
	int botCols = m_bot.cols;
	int prevBotRows = m_prevBot.rows;
	int prevBotCols = m_prevBot.cols;

	bool topMatch = topRows == prevTopRows && topCols == prevTopCols;
	bool botMatch = botRows == prevBotRows && botCols == prevBotCols;

	if (!topMatch || !botMatch)
	{
		//std::cout << "Sizes don't match.\n";
		m_top.copyTo(m_prevTop);
		m_bot.copyTo(m_prevBot);
		return;
	}

	// Subtract the prev frame
	cv::Mat topDiff;
	cv::Mat botDiff;
	cv::absdiff(m_top, m_prevTop, topDiff);
	cv::absdiff(m_bot, m_prevBot, botDiff);

	//Rescale diffs to be on 0 to 1
	topDiff.convertTo(topDiff, CV_16UC1, 1.0 / 255);
	botDiff.convertTo(botDiff, CV_16UC1, 1.0 / 255);

	int topSum = static_cast<int>(cv::sum(topDiff)[0]);
	int botSum = static_cast<int>(cv::sum(botDiff)[0]);

	m_isMoving = topSum > 0 && botSum > 0;
	//if (m_isMoving && m_player == 0) std::cout << m_isMoving << "\n";

	m_top.copyTo(m_prevTop);
	m_bot.copyTo(m_prevBot);
	
	//double min, max;
	//cv::Point min_loc, max_loc;
	//cv::minMaxLoc(rowSums, &min, &max, &min_loc, &max_loc);
	////std::cout << " ||| Min: " << min_loc.y << " // " << "Max: " << max_loc.y << "\n\n\n\n";
}

std::vector<cv::Rect> NextTracker::getROIs()
{
	std::vector<cv::Rect> rects(2);
	rects[0] = m_topRoi;
	rects[1] = m_botRoi;
	return rects;
}

cv::Rect NextTracker::nextROIFromField(cv::Rect& medianField)
{
	cv::Rect result;
	if (m_player == 0)
	{
		int x{ static_cast<int>(medianField.x + medianField.width * 1.08) };
		int y{ static_cast<int>(medianField.y - medianField.height * 0.02) };
		int w{ static_cast<int>(medianField.width * 0.35) };
		int h{ static_cast<int>(medianField.height * 0.40) };
		result = cv::Rect(x, y, w, h);
	}
	else if (m_player == 1)
	{
		int x{ static_cast<int>(medianField.x - medianField.width * 0.08 - medianField.width * 0.35) };
		int y{ static_cast<int>(medianField.y - medianField.height * 0.02) };
		int w{ static_cast<int>(medianField.width * 0.35) };
		int h{ static_cast<int>(medianField.height * 0.40) };
		result = cv::Rect(x, y, w, h);
	}
	return result;
}

void NextTracker::setTopBotROIs(cv::Rect& nextROI)
{
	//if (m_player == 0)
	//{
	//	int topX{ static_cast<int>(nextROI.x + nextROI.width * 0.08) };
	//	int topY{ static_cast<int>(nextROI.y + nextROI.height * 0.05) };
	//	int topW{ static_cast<int>(nextROI.width * 0.5) };
	//	int topH{ static_cast<int>(nextROI.height * 0.1) };
	//	m_topRoi = cv::Rect(topX, topY, topW, topH);

	//	int botX{ static_cast<int>(nextROI.x + nextROI.width * 0.40) };
	//	int botY{ static_cast<int>(nextROI.y + nextROI.height * 0.80) };
	//	int botW{ static_cast<int>(nextROI.width * 0.5) };
	//	int botH{ static_cast<int>(nextROI.height * 0.1) };
	//	m_botRoi = cv::Rect(botX, botY, botW, botH);
	//}
	//else if (m_player == 1)
	//{
	//	int topX{ static_cast<int>(nextROI.x + nextROI.width * 0.92 - nextROI.width * 0.5) };
	//	int topY{ static_cast<int>(nextROI.y + nextROI.height * 0.05) };
	//	int topW{ static_cast<int>(nextROI.width * 0.5) };
	//	int topH{ static_cast<int>(nextROI.height * 0.1) };
	//	m_topRoi = cv::Rect(topX, topY, topW, topH);

	//	int botX{ static_cast<int>(nextROI.x + nextROI.width * 0.05) };
	//	int botY{ static_cast<int>(nextROI.y + nextROI.height * 0.80) };
	//	int botW{ static_cast<int>(nextROI.width * 0.5) };
	//	int botH{ static_cast<int>(nextROI.height * 0.1) };
	//	m_botRoi = cv::Rect(botX, botY, botW, botH);
	//}

	if (m_player == 0)
	{
		int topX{ static_cast<int>(nextROI.x + nextROI.width * 0.16) };
		int topY{ static_cast<int>(nextROI.y + nextROI.height * 0.13) };
		int topW{ static_cast<int>(nextROI.width * 0.36) };
		int topH{ static_cast<int>(nextROI.height * 0.13) };
		m_topRoi = cv::Rect(topX, topY, topW, topH);

		int botX{ static_cast<int>(nextROI.x + nextROI.width * 0.53) };
		int botY{ static_cast<int>(nextROI.y + nextROI.height * 0.75) };
		int botW{ static_cast<int>(nextROI.width * 0.26) };
		int botH{ static_cast<int>(nextROI.height * 0.09) };
		m_botRoi = cv::Rect(botX, botY, botW, botH);
	}
	else if (m_player == 1)
	{
		int topX{ static_cast<int>(nextROI.x + nextROI.width * 0.50) };
		int topY{ static_cast<int>(nextROI.y + nextROI.height * 0.13) };
		int topW{ static_cast<int>(nextROI.width * 0.36) };
		int topH{ static_cast<int>(nextROI.height * 0.13) };
		m_topRoi = cv::Rect(topX, topY, topW, topH);

		int botX{ static_cast<int>(nextROI.x + nextROI.width * 0.23) };
		int botY{ static_cast<int>(nextROI.y + nextROI.height * 0.75) };
		int botW{ static_cast<int>(nextROI.width * 0.26) };
		int botH{ static_cast<int>(nextROI.height * 0.09) };
		m_botRoi = cv::Rect(botX, botY, botW, botH);
	}
}

bool& NextTracker::isMoving()
{
	return m_isMoving;
}