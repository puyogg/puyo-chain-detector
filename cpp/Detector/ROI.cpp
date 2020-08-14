#include <opencv2/opencv.hpp>

void getScreenRects(cv::UMat grayblur)
{
	cv::UMat mask;
	cv::compare(grayblur, 81, mask, cv::CMP_LE);

}