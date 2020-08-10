#pragma once
#include <iostream>
#include <chrono>
#include <deque>

using milli = std::chrono::milliseconds;

class FPS
{
public:
	FPS();
	void tick();
private:
	std::chrono::steady_clock::time_point m_prev;
	std::deque<long long> frameQueue{};
	int m_ticks;
};

