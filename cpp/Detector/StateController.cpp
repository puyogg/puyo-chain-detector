#include "StateController.h"

StateController::StateController(int player, cv::dnn::Net& net, bool tryIgnorePopping)
{
	m_player = player;
	m_net = net;
	m_tryIgnorePopping = tryIgnorePopping;

	for (int y = 0; y < 12; y++)
	{
		for (int x = 0; x < 6; x++)
		{
			int xPos{ x * 32 };
			int yPos{ y * 30 };
			cv::Rect rect{ xPos, yPos, 32, 30 };
			m_cellROIs[(int64_t)y * 6 + (int64_t)x] = rect;
		}
	}
}

void StateController::update(cv::Mat& frame, ROIController& roiController, bool reset)
{
	PlayerROIs player = roiController.player(m_player);
	
	// Check to see if the screen faded to black?
	// Pass in an update variable from Detector.cpp
	if (reset)
	{
		//std::cout << "Screen turned to black.\n";
		m_cursorData.clear();
		return;
	}

	// Check if the X was detected in the score region. That means Puyo are popping.
	// When Puyos are popping, there's no need to display the data.
	if (player.scoreX().hasX())
	{
		// Clear cursor data
		//std::cout << "Player" << m_player << " saw the X.\n";
		m_cursorData.clear();
		return;
	}

	// If the Next Window is moving, that means a Puyo was just placed.
	// This is when we should run the chain analyzers.
	if (player.nextWindow().isMoving() && player.field().getROI().area() > 0)
	{
		//std::cout << m_player << ": Next moved!\n";
		cv::Mat cropCharBG = frame(player.field().getROI());
		cv::Mat resized;
		cv::resize(cropCharBG, resized, cv::Size(200, 362), 0, 0, cv::INTER_LINEAR);
		// Normalize values from [0, 255] to [0, 1]
		resized.convertTo(resized, CV_32FC3, 1.0 / 255.0);
		m_centerCrop = resized(cv::Rect(4, 1, 192, 360));
		setCellMats();
		predictField(); // updates m_field
		Chainsim::PuyoField puyoField;
		puyoField.set(m_field);

		// If m_tryIgnorePops was set to true via settings.json, check for pops first
		if (m_tryIgnorePopping)
		{
			auto [hasPops, popCounts, popColors, popPositions] = puyoField.checkPops();
			// Return, but don't clear away the previous analysis if there was one.
			if (hasPops) return;
		}

		// Try out chains
		m_cursorData = puyoField.searchForChains();
		//std::cout << "Found " << m_cursorData.size() << " chains to try.\n";
		//for (auto& s : m_cursorData)
		//{
		//	auto [r, c, color, length] = s;
		//	std::cout << r << ", " << c << ", " << color << ", " << length << ";\n";
		//}
	}
}

void StateController::setCellMats()
{
	for (int64_t i = 0; i < 72; i++)
	{
		m_cellMats[i] = m_centerCrop(m_cellROIs[i]);
	}
}

void StateController::predictField()
{
	// Create blob to feed into neural network. Oreded by NCHW
	cv::Mat blob = cv::dnn::blobFromImages(m_cellMats, 2.0, cv::Size(32, 30), cv::Scalar(0.5, 0.5, 0.5), true, false, CV_32F);
	m_net.setInput(blob);
	cv::Mat prob = m_net.forward();
	cv::Mat sort;
	cv::sortIdx(prob, sort, cv::SORT_EVERY_ROW + cv::SORT_DESCENDING);
	std::vector<int64_t> field{ 0, 0, 0, 0, 0, 0 }; // Pad hidden row
	for (int i = 0; i < 72; i++)
	{
		field.push_back(static_cast<int64_t>(sort.at<int>(i, 0)));
	}
	m_field = field;
}

std::vector<int64_t> StateController::field()
{
	return m_field;
}

std::vector<std::tuple<int64_t, int64_t, Chainsim::Color, int64_t>>& StateController::cursorData()
{
	return m_cursorData;
}
