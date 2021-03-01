#include "PlayerTracker.hpp"
#include "Constants.hpp"
#include <opencv2/imgproc.hpp>
#include <iostream>

using namespace ChainDetector;
using CursorData = std::vector<std::tuple<Chainsim::Pos, Chainsim::Color, int>>;

PlayerTracker::PlayerTracker(int player, cv::dnn::Net& net, CaptureSettings& captureSettings)
    : m_nextTracker(player, captureSettings), m_scoreTracker(player, captureSettings)
{
    m_player = player;
    m_net = &net;
    m_settings = &captureSettings;

    // Precalculate the field cell rects
    for (int y = 0; y < 12; y++)
    {
        for (int x = 0; x < 6; x++)
        {
            int xPos{ x * PUYO_W };
            int yPos{ y * PUYO_H };
            cv::Rect rect{ xPos, yPos, PUYO_W, PUYO_H };
            m_cellRects[y * 6 + x] = rect;
        }
    }
}

void PlayerTracker::update(cv::Mat &frame)
{
    cv::Rect& fieldRect = m_settings->player.at(m_player).fieldRect;

    // Check if the score window is implying that a chain
    // is popping. In that case, reset the fields
    if (m_scoreTracker.isPopping(frame))
    {
        m_lengthField.reset();
        m_popColors.reset();
    }
    // Check if next window is moving
    else if (m_nextTracker.isMoving(frame))
    {
        // Crop the field from the frame
        cv::Mat fieldMat = frame(fieldRect);

        // Change value range from [0, 255] to [0, 1] (float)
        fieldMat.convertTo(fieldMat, CV_32FC3, 1.0 / 255.0);

        std::vector<Chainsim::Color> fieldColors = readFieldColors(fieldMat);
        Chainsim::PuyoField puyoField;
        puyoField.set(fieldColors);
        puyoField.copyTo(m_puyoField);

        // If this field is already popping, ignore it
        auto [hasPops, popCounts, popColors, popPositions] = puyoField.checkPops();
        if (!hasPops)
        {
            // Try out chains
            CursorData cursorData = puyoField.searchForChains();
    //        std::cout << "Found " << cursorData.size() << " chains to try.\n";
    //        for (auto& s : cursorData)
    //        {
    //            auto [pos, color, length] = s;
    //            std::cout << pos.r << ", " << pos.c << ", " << static_cast<int>(color) << ", " << length << ";\n";
    //        }
    //        std::cout << "\n";

            // Reset puyoField and lengthField
            m_lengthField.reset();
            m_popColors.reset();

            // Set the cursorData into the fields
            for (auto& data: cursorData)
            {
                auto [pos, color, length] = data;
                if (length > m_lengthField.get(pos.r, pos.c))
                {
                    m_lengthField.set(pos.r, pos.c, length);
                    m_popColors.set(pos.r, pos.c, color);
                }
            }
        }
    }

    // Edit the original frame if enablePreview is true in capture settings
    if (m_settings->enablePreview)
    {
        cv::rectangle(frame, fieldRect, cv::Scalar(0, 0, 255), 2);
        cv::rectangle(frame, m_settings->player.at(m_player).scoreRect, cv::Scalar(0, 0, 255), 2);
        cv::rectangle(frame, m_settings->player.at(m_player).bonusRect, cv::Scalar(0, 0, 255), 1);
        for (cv::Rect rect : m_settings->player.at(m_player).nextRects)
        {
            cv::rectangle(frame, rect, cv::Scalar(0, 0, 255), 2);
        }
    }
}


std::vector<Chainsim::Color> PlayerTracker::readFieldColors(cv::Mat& fieldMat)
{
    // Get crops for each field cell
    std::vector<cv::Mat> cellMats{ 72 };
    for (int i = 0; i < 72; i++)
    {
        cellMats[i] = fieldMat(m_cellRects[i]);
    }

    // Create a blob to feed into the neural network, ordered by NCHW
    cv::Mat blob = cv::dnn::blobFromImages(cellMats, 2.0, cv::Size(PUYO_W, PUYO_H), cv::Scalar(0.5, 0.5, 0.5), true, false, CV_32F);
    m_net->setInput(blob);
    cv::Mat prob = m_net->forward();
    cv::Mat sort;
    cv::sortIdx(prob, sort, cv::SORT_EVERY_ROW + cv::SORT_DESCENDING);
    std::vector<Chainsim::Color> fieldColors{ 78 };
    for (int i = 0; i < 6; i++)
    {
        // Pad hidden row
//        fieldColors.push_back(Chainsim::Color::NONE);
        fieldColors.at(i) = Chainsim::Color::NONE;
    }
    for (int i = 0; i < 72; i++)
    {
//        fieldColors.push_back(static_cast<Chainsim::Color>(sort.at<int>(i, 0)));
        fieldColors.at(i + 6) = static_cast<Chainsim::Color>(sort.at<int>(i, 0));
    }

    return fieldColors;
}
