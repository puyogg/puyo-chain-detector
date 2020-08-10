#pragma once
#include "Constants.h"
#include <vector>

class CountField
{
public:
	std::vector<int64_t> m_data;
	int64_t m_rows;
	int64_t m_cols;

	CountField(int64_t rows = DEFAULT_ROWS, int64_t cols = DEFAULT_COLS);

	int64_t get(int64_t row, int64_t col);
	void set(int64_t row, int64_t col, int64_t n);
	void reset();
	void print();
};

