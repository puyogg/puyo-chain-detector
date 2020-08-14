#pragma once
#include <opencv2/opencv.hpp>
#include <cstddef>

constexpr std::size_t DEFAULT_COLS{ 6 };
constexpr std::size_t DEFAULT_ROWS{ 13 };
constexpr std::size_t DEFAULT_HROWS{ 1 };
constexpr int DEFAULT_PUYOTOPOP{ 4 };

constexpr double VIDEO_HEIGHT = 270.0 * 2;
constexpr double VIDEO_WIDTH = 480.0 * 2;
const cv::Rect P0_FIELD{ 136, 79, 200, 362 };
const cv::Rect P1_FIELD{ 624, 79, 200, 362 };
constexpr double EXPECTED_AR{ (double)200 / 362 };