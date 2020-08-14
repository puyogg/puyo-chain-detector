#pragma once
#include "Constants.h"
#include <vector>


namespace Chainsim {

	class BoolField
	{
	public:
		std::vector<bool> m_data;
		int64_t m_rows;
		int64_t m_cols;

		BoolField(int64_t rows = DEFAULT_ROWS, int64_t cols = DEFAULT_COLS);

		bool get(int64_t row, int64_t col);
		void set(int64_t row, int64_t col, bool b);
		void reset();
		void print();
	};

}