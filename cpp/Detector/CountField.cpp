#include "CountField.h"

CountField::CountField(int64_t rows, int64_t cols)
{
	m_data = std::vector<int64_t>(rows * cols, false);
	m_rows = rows;
	m_cols = cols;
}

int64_t CountField::get(int64_t row, int64_t col)
{
	return m_data[row * m_cols + col];
}

void CountField::set(int64_t row, int64_t col, int64_t n)
{
	m_data[row * m_cols + col] = n;
}

void CountField::reset()
{
	std::fill(m_data.begin(), m_data.end(), 0);
}

void CountField::print()
{
	for (int64_t r = 0; r < m_rows; ++r)
	{
		for (int64_t c = 0; c < m_cols; ++c) {
			std::cout << static_cast<int64_t>(get(r, c));
		}
		std::cout << '\n';
	}
}