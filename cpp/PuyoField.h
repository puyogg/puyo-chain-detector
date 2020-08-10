#pragma once
#include <vector>
#include <tuple>
#include <set>
#include "Constants.h"
#include "BoolField.h"

//using Color = std::uint_fast8_t;
using Color = int64_t;
struct Pos {
	int64_t r;
	int64_t c;
};

using PopCounts = std::vector<int64_t>;
using PopColors = std::set<int64_t>;
using PopPositions = std::vector<Pos>;
using PopData = std::tuple<bool, PopCounts, PopColors, PopPositions>;

class PuyoField
{
public:
	PuyoField(int64_t rows = DEFAULT_ROWS, int64_t cols = DEFAULT_COLS, int64_t hrows = DEFAULT_HROWS, int64_t puyoToPop = DEFAULT_PUYOTOPOP);

	Color get(int64_t row, int64_t col);
	void set(int64_t row, int64_t col, Color color);
	void set(std::vector<Color>& vector1d);
	void set(std::vector<std::vector<Color>>& vector2d);
	void copyTo(PuyoField& field);
	int64_t simulate();
	std::vector<std::tuple<int64_t, int64_t, Color, int64_t>> searchForChains();
	void print();

private:
	BoolField m_bool;
	std::vector<Color> m_data;
	int64_t m_rows;
	int64_t m_cols;
	int64_t m_hrows;
	int64_t m_puyoToPop;
	int64_t m_chainLength;

	void dropPuyos();
	PopData checkPops();
	void applyPops(PopPositions& positions);
	void removeDrops();
	std::vector<std::vector<int64_t>> surfaceInds();
	std::vector<std::tuple<int64_t, int64_t, Color>> tryColorsAtInds(std::vector<std::vector<int64_t>>& inds);
	std::vector<PuyoField> fieldsToTry(std::vector<std::tuple<int64_t, int64_t, Color>>& colorsAtInds);
};