#include "BoolField.h"
#include <iostream>
#include <vector>

namespace Chainsim {
	BoolField::BoolField(int64_t rows, int64_t cols)
	{
		m_data = std::vector<bool>(rows * cols, false);
		m_rows = rows;
		m_cols = cols;
	}

	bool BoolField::get(int64_t row, int64_t col)
	{
		return m_data[row * m_cols + col];
	}

	void BoolField::set(int64_t row, int64_t col, bool b)
	{
		m_data[row * m_cols + col] = b;
	}

	void BoolField::reset()
	{
		std::fill(m_data.begin(), m_data.end(), 0);
	}

	void BoolField::print()
	{
		for (int64_t r = 0; r < m_rows; ++r)
		{
			for (int64_t c = 0; c < m_cols; ++c) {
				std::cout << static_cast<int64_t>(get(r, c));
			}
			std::cout << '\n';
		}
	}
}