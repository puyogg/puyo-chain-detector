#include "FieldTracker.h"

double euclideanDist(std::vector<double> vec)
{
	double sum{ 0 };
	for (double v : vec)
	{
		sum += v * v;
	}
	return sqrt(sum);
}

FieldTracker::FieldTracker(size_t historySize)
{
	m_historySize = historySize;
}

FieldTracker::FieldTracker(int player, size_t historySize)
{
	m_historySize = historySize;
	
	if (player == 0)
	{
		//cv::Rect rect{ 68, 39, 100, 180 };
		cv::Rect rect{ P0_FIELD.x, P0_FIELD.y, P0_FIELD.width, P0_FIELD.height };
		m_history.push_back(rect);
		m_default.x = rect.x;
		m_default.y = rect.y;
		m_default.width = rect.width;
		m_default.height = rect.height;
	}
	else if (player == 1)
	{
		//cv::Rect rect{ 312, 39, 100, 180 };
		cv::Rect rect{ P1_FIELD.x, P1_FIELD.y, P1_FIELD.width, P1_FIELD.height };
		m_history.push_back(rect);
		m_default.x = rect.x;
		m_default.y = rect.y;
		m_default.width = rect.width;
		m_default.height = rect.height;
	}
}

void FieldTracker::update(std::vector<cv::Rect>& fieldROI, cv::Mat& roiAnalysis)
{
	// If fieldROI is empty, return. In the following call to getROI, FieldTracker
	// will just give the last known ROI
	if (fieldROI.size() == 0) return;

	// Compute Euclidean distance to expected x,y,w,h
	std::vector<std::vector<double>> incomingNorm;
	for (auto& f : fieldROI)
	{
		double vNormX{ static_cast<double>(f.x) / m_default.x };
		double vNormY{ static_cast<double>(f.y) / m_default.y };
		double vNormW{ static_cast<double>(f.width) / m_default.width };
		double vNormH{ static_cast<double>(f.height) / m_default.height };
		std::vector<double> vec{ vNormX, vNormY, vNormW, vNormH };
		incomingNorm.push_back(vec);
	}
	// Find the vector in incomingNorm that has the highest similarity to m_default
	std::vector<double> dists;
	for (auto& vec : incomingNorm)
	{
		for (auto& v : vec)
		{
			v -= 1.0;
		}
		dists.push_back(euclideanDist(vec));
	}
	// Find the min index
	std::vector<double>::iterator iter = std::min_element(dists.begin(), dists.end());

	// Calculate similarity score
	// If the similarity score is at least 80%, then this ROI is probably the correct field.
	// 82% was chosen to adjust for the fact that some users don't have their console screen set to the full screen area.
	// For example, some people have to run the game at 96% to make it fit within their monitor's side bars.
	double similarity{ 1 / (1 + *iter) };
	if (similarity > 0.82) {
		// https://stackoverflow.com/a/9256414 Getting index from iterator
		int64_t idx{ iter - dists.begin() };
		cv::Rect& field = fieldROI[idx];
		m_activeROI = field;

		m_history.push_back(field);
	}

	if (m_history.size() >= m_historySize) {
		m_history.pop_front();
	}
}

cv::Rect& FieldTracker::getROI()
{
	return m_activeROI;
}

cv::Rect FieldTracker::getMedian()
{
	std::vector<int> xVals;
	std::vector<int> yVals;
	std::vector<int> wVals;
	std::vector<int> hVals;

	for (auto& h : m_history)
	{
		xVals.push_back(h.x);
		yVals.push_back(h.y);
		wVals.push_back(h.width);
		hVals.push_back(h.height);
	}

	// Sort the median element to the middle
	std::nth_element(xVals.begin(), xVals.begin() + xVals.size() / 2, xVals.end());
	std::nth_element(yVals.begin(), yVals.begin() + yVals.size() / 2, yVals.end());
	std::nth_element(wVals.begin(), wVals.begin() + wVals.size() / 2, wVals.end());
	std::nth_element(hVals.begin(), hVals.begin() + hVals.size() / 2, hVals.end());

	return cv::Rect(xVals[xVals.size() / 2], yVals[yVals.size() / 2], wVals[wVals.size() / 2], hVals[hVals.size() / 2]);
}

int FieldTracker::getMaxY()
{
	std::vector<int> yVals;
	for (auto& h : m_history)
	{
		yVals.push_back(h.y);
	}

	return *std::max_element(yVals.begin(), yVals.end());
}