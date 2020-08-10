#include "ROIController.h"

cv::Mat roiAnalysisKernel{ cv::getStructuringElement(cv::MORPH_RECT, cv::Size{ 3, 3 }) };
cv::Mat labels;
cv::Mat stats;
cv::Mat centroids;

ROIController::ROIController()
{
    m_player0 = PlayerROIs(0);
    m_player1 = PlayerROIs(1);
}

void ROIController::update(cv::Mat& frame, cv::Mat& roiAnalysis, cv::Mat& nextAnalysis, cv::Mat& scoreAnalysis)
{
    // Find the apparent field ROIs
    std::vector<cv::Rect> validFields = findFields(frame, roiAnalysis);

    // Threshold Next Window
    cv::compare(nextAnalysis, 180, nextAnalysis, cv::CMP_GE);

    // Threshold Score Window
    cv::compare(scoreAnalysis, 200, scoreAnalysis, cv::CMP_GT);

    // Send the ROIs to each player's tracker, and they'll each figure out if its valid or not
    m_player0.update(validFields, roiAnalysis, nextAnalysis, scoreAnalysis);
    m_player1.update(validFields, roiAnalysis, nextAnalysis, scoreAnalysis);

    //m_player0.drawROIs(frame);
    //m_player1.drawROIs(frame);
}

std::vector<cv::Rect> ROIController::findFields(cv::Mat& frame, cv::Mat& roiAnalysis)
{
    // Get necessary full image conversions
    //cv::GaussianBlur(roiAnalysis, roiAnalysis, cv::Size{ 3, 3 }, 0);
    cv::GaussianBlur(roiAnalysis, roiAnalysis, cv::Size{ 5, 5 }, 0);
    // Threshold Game Field
    cv::compare(roiAnalysis, 81, roiAnalysis, cv::CMP_LE);
    //cv::compare(roiAnalysis, 100, roiAnalysis, cv::CMP_LE);
    // Binary closing to merge nearby contours
    cv::morphologyEx(roiAnalysis, roiAnalysis, cv::MORPH_CLOSE, roiAnalysisKernel);

    // Find contours
    cv::connectedComponentsWithStats(roiAnalysis, labels, stats, centroids);
    std::vector<cv::Rect> validFields{};
    int maxY0 = m_player0.fieldMaxY();
    int maxY1 = m_player1.fieldMaxY();
    for (int i = 0; i < stats.rows; ++i)
    {
        int x{ stats.at<int>(i, 0) };
        int y{ stats.at<int>(i, 1) };
        int w{ stats.at<int>(i, 2) };
        int h{ stats.at<int>(i, 3) };

        // Ignore components that are smaller than 8% of the full screen's area.
        if ((double)w * h < (0.08 * frame.size().height * frame.size().width)) {
            continue;
        }

        // Ignore components that are more than 5% different than the expected aspect ratio
        double aspectRatio{ (double)w / h };
        double pDiff{ std::abs((aspectRatio - EXPECTED_AR) / ((aspectRatio + EXPECTED_AR) / 2)) };
        if (pDiff > 0.05) continue;

        // Assuming the width is correct, reproject the expected aspect ratio.
        // Redraw the ROI bottom up.
        int wantedHeight = static_cast<int>(round(static_cast<double>(w) / EXPECTED_AR));
        int yBottom = y + h;
        int trueTop = yBottom - wantedHeight;

        // Create Rects for the valid contours
        cv::Rect rect{ x, trueTop, w, wantedHeight };

        validFields.push_back(rect);
    }

    return validFields;
}

PlayerROIs& ROIController::player(int id)
{
    return id == 0 ? m_player0 : m_player1;
}