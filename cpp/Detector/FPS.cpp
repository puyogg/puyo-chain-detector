#include "FPS.h"

FPS::FPS()
{
	m_prev = std::chrono::high_resolution_clock::now();
	m_ticks = 0;
}

void FPS::tick()
{
	auto t = std::chrono::high_resolution_clock::now();
	frameQueue.push_back(std::chrono::duration_cast<milli>(t - m_prev).count());
	m_prev = t;

	m_ticks = (m_ticks + 1) % 60;
	if (m_ticks == 59)
	{
		double count{ 0.0 };
		double sum{ 0.0 };
		for (auto& v : frameQueue) {
			sum += v;
			count += 1.0;
		}
		double mean = sum / count;
		std::cout << "Average FPS: " << 60 / (mean / 16.6667) << std::endl;
	}

	if (frameQueue.size() == 60)
	{
		frameQueue.pop_front();
	}
}
