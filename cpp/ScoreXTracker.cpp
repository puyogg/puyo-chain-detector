#include "ScoreXTracker.h"

//cv::Mat xTemplate = cv::imread("score-x-template.png", cv::IMREAD_GRAYSCALE).getUMat(cv::ACCESS_READ);
cv::Mat xTemplate = cv::imread("score-x-template.png", cv::IMREAD_GRAYSCALE);

ScoreXTracker::ScoreXTracker()
{
	m_player = 0;
	m_hasX = false;
}

ScoreXTracker::ScoreXTracker(int player)
{
	m_player = player;
	m_hasX = false;
}

void ScoreXTracker::update(cv::Rect& medianField, cv::Mat& scoreAnalysis)
{
	// Get the bounding box for the score region
	cv::Rect scoreROI{ scoreROIFromField(medianField) };
	m_scoreROI = scoreROI;

	// Apply threshold to score region
	cv::Mat scoreRegion = scoreAnalysis(scoreROI);
	cv::compare(scoreRegion, 200, scoreRegion, cv::CMP_GT);

	// The template matching wont work at different
	// screen sizes unless I resize the score x...
	double scaling = (double)medianField.width / P0_FIELD.width;
	int newWidth = static_cast<int>((double)xTemplate.cols * scaling);
	int newHeight = static_cast<int>((double)xTemplate.rows * scaling);
	cv::Mat xScaled;
	int method = scaling < 1 ? cv::INTER_AREA : cv::INTER_LINEAR;
	cv::resize(xTemplate, xScaled, cv::Size(newWidth, newHeight), 0, 0, method);
	//std::cout << cv::Size(newWidth, newHeight) << '\n';

	// Template Match
	cv::Mat result;
	cv::Mat score = scoreAnalysis(scoreROI);
	cv::matchTemplate(score, xScaled, result, cv::TM_CCOEFF_NORMED);
	double minVal;
	double maxVal;
	cv::Point minLoc;
	cv::Point maxLoc;
	cv::minMaxLoc(result, &minVal, &maxVal, &minLoc, &maxLoc);

	//std::cout << "Player " << m_player << " match value: " << maxVal << "\n";
	if (newHeight == xTemplate.rows && newWidth == xTemplate.cols)
	{
		m_hasX = maxVal > 0.80;
	}
	else
	{
		m_hasX = maxVal > 0.70; // tested at 96% screen area
	}
}

cv::Rect& ScoreXTracker::getROI()
{
	return m_scoreROI;
}

cv::Rect ScoreXTracker::scoreROIFromField(cv::Rect& medianField)
{
	/*int x{ static_cast<int>(medianField.x) };
	int y{ static_cast<int>(medianField.y + medianField.height) };
	int w{ static_cast<int>(medianField.width * 1.02) };
	int h{ static_cast<int>(medianField.height / 11) };

	if (m_player == 1)
	{
		x -= static_cast<int>(medianField.width * 0.02);
	}
	return cv::Rect(x, y, w, h);*/
	int x{ static_cast<int>(medianField.x) };
	int y{ static_cast<int>(medianField.y + medianField.height) };
	int w{ static_cast<int>(medianField.width * 0.3) };
	int h{ static_cast<int>(medianField.height / 12) };

	if (m_player == 0)
	{
		x += static_cast<int>(medianField.width * 0.50);
		y += static_cast<int>(medianField.height * 0.01);
	}
	else
	{
		x += static_cast<int>(medianField.width * 0.30);
		y += static_cast<int>(medianField.height * 0.01);
	}

	return cv::Rect(x, y, w, h);
}

bool& ScoreXTracker::hasX()
{
	return m_hasX;
}