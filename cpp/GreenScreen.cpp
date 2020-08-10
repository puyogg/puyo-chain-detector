#include "GreenScreen.h"

GreenScreen::GreenScreen()
{
	// Set screen to green
	m_screen = cv::Mat(540, 960, CV_8UC4, cv::Scalar(0, 255, 0, 255));

	// Set alpha layers
	m_cursorAlpha = cv::Mat(540, 960, CV_8UC4, cv::Scalar(0, 0, 0, 0));
	m_numberAlpha = cv::Mat(540, 960, CV_8UC4, cv::Scalar(0, 0, 0, 0));

	// Load cursors and numbers
	m_cursors.push_back(cv::imread("green-screen/red_cursor.png", cv::IMREAD_UNCHANGED));
	m_cursors.push_back(cv::imread("green-screen/red_cursor.png", cv::IMREAD_UNCHANGED));
	m_cursors.push_back(cv::imread("green-screen/red_cursor.png", cv::IMREAD_UNCHANGED));
	m_cursors.push_back(cv::imread("green-screen/green_cursor.png", cv::IMREAD_UNCHANGED));
	m_cursors.push_back(cv::imread("green-screen/blue_cursor.png", cv::IMREAD_UNCHANGED));
	m_cursors.push_back(cv::imread("green-screen/yellow_cursor.png", cv::IMREAD_UNCHANGED));
	m_cursors.push_back(cv::imread("green-screen/purple_cursor.png", cv::IMREAD_UNCHANGED));

	m_numbers.push_back(cv::imread("green-screen/2.png", cv::IMREAD_UNCHANGED));
	m_numbers.push_back(cv::imread("green-screen/2.png", cv::IMREAD_UNCHANGED));
	m_numbers.push_back(cv::imread("green-screen/2.png", cv::IMREAD_UNCHANGED));
	m_numbers.push_back(cv::imread("green-screen/3.png", cv::IMREAD_UNCHANGED));
	m_numbers.push_back(cv::imread("green-screen/4.png", cv::IMREAD_UNCHANGED));
	m_numbers.push_back(cv::imread("green-screen/5.png", cv::IMREAD_UNCHANGED));
	m_numbers.push_back(cv::imread("green-screen/6.png", cv::IMREAD_UNCHANGED));
	m_numbers.push_back(cv::imread("green-screen/7.png", cv::IMREAD_UNCHANGED));
	m_numbers.push_back(cv::imread("green-screen/8.png", cv::IMREAD_UNCHANGED));
	m_numbers.push_back(cv::imread("green-screen/9.png", cv::IMREAD_UNCHANGED));
	cv::Mat omg = cv::imread("green-screen/omg.png", cv::IMREAD_UNCHANGED);
	for (int i = 0; i < 11; i++)
	{
		m_numbers.push_back(omg);
	}

	for (auto& c : m_cursors)
	{
		cv::resize(c, c, cv::Size(32, 30), 0, 0, cv::INTER_LINEAR);
	}

	for (auto& n : m_numbers)
	{
		cv::resize(n, n, cv::Size(32, 30), 0, 0, cv::INTER_LINEAR);
	}

	// Set up ROIs for drawing in the alpha layer
	for (int r = 0; r < 13; r++)
	{
		for (int c = 0; c < 6; c++)
		{
			m_p0ROIs.push_back(cv::Rect(280 / 2 + (c * 32), 50 + (r * 30), 32, 30));
			m_p1ROIs.push_back(cv::Rect(1256 / 2 + (c * 32), 50 + (r * 30), 32, 30));
		}
	}
}

cv::Mat& GreenScreen::screen()
{
	return m_screen;
}

void GreenScreen::update(std::vector<std::tuple<int64_t, int64_t, Color, int64_t>>& p0Data, std::vector<std::tuple<int64_t, int64_t, Color, int64_t>>& p1Data)
{
	m_countField.reset();
	resetScreen(); // Reset Green Screen

	for (auto& data : p0Data)
	{
		auto [r, c, color, length] = data;
		cv::Rect roi = m_p0ROIs[r * 6 + c];
		cv::Mat cursor = m_cursors[color];
		cv::Mat number = m_numbers[length];
		cursor.copyTo(m_cursorAlpha(roi));
		number.copyTo(m_numberAlpha(roi));
	}

	for (auto& data : p1Data)
	{
		auto [r, c, color, length] = data;
		cv::Rect roi = m_p1ROIs[r * 6 + c];
		cv::Mat cursor = m_cursors[color];
		cv::Mat number = m_numbers[length];
		cursor.copyTo(m_cursorAlpha(roi));
		number.copyTo(m_numberAlpha(roi));
	}

	cv::Mat cursorMask;
	cv::Mat numberMask;
	cv::extractChannel(m_cursorAlpha, cursorMask, 3);
	cv::extractChannel(m_numberAlpha, numberMask, 3);
	m_cursorAlpha.copyTo(m_screen, cursorMask);
	m_numberAlpha.copyTo(m_screen, numberMask);
}

void GreenScreen::resetScreen()
{
	m_screen.setTo(cv::Scalar(0, 255, 0, 255));
	m_cursorAlpha.setTo(cv::Scalar(0, 0, 0, 0));
	m_numberAlpha.setTo(cv::Scalar(0, 0, 0, 0));
}