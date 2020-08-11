#include "PuyoField.h"
#include "BoolField.h"
#include <iostream>
#include <vector>
#include <tuple>
#include <queue>

PuyoField::PuyoField(int64_t rows, int64_t cols, int64_t hrows, int64_t puyoToPop)
{
	m_rows = static_cast<int64_t>(rows);
	m_cols = static_cast<int64_t>(cols);
	m_data = std::vector<Color>(m_rows * m_cols, static_cast<Color>(0));
	m_hrows = hrows;
	m_puyoToPop = puyoToPop;
	m_chainLength = 0;
}

Color PuyoField::get(int64_t row, int64_t col) {
	return m_data[row * m_cols + col];
}

void PuyoField::set(int64_t row, int64_t col, Color color) {
	m_data[row * m_cols + col] = color;
}

void PuyoField::set(std::vector<Color>& vector1d)
{
	for (int64_t r = 0; r < m_rows; r++)
	{
		for (int64_t c = 0; c < m_cols; c++)
		{
			set(r, c, vector1d[r * m_cols + c]);
		}
	}
}

void PuyoField::set(std::vector<std::vector<Color>>& vector2d) {
	int64_t v_rows = static_cast<int64_t>(vector2d.size());
	int64_t v_cols = static_cast<int64_t>(vector2d[0].size());

	for (int64_t r = 0; r < v_rows; ++r)
	{
		for (int64_t c = 0; c < v_cols; ++c)
		{
			set(r, c, vector2d[r][c]);
		}
	}
}

void PuyoField::copyTo(PuyoField& field) {
	for (int64_t r = 0; r < m_rows; ++r)
	{
		for (int64_t c = 0; c < m_cols; ++c)
		{
			//set(r, c, field.get(r, c));
			field.set(r, c, get(r, c));
		}
	}
}

void PuyoField::dropPuyos() {
	// In each column...
	for (int64_t c = 0; c < m_cols; ++c)
	{
		int64_t end = m_rows - 1;

		// Traverse the rows bottom up
		for (int64_t r = end; r >= 0; --r)
		{
			Color current = get(r, c);

			// If the current cell contains any kind of Puyo...
			if (current > 0)
			{
				// Move it to the cell currently marked as "end"
				set(end, c, current);
				// Empty the current cell
				if (r != end) set(r, c, 0);
				// Shift "end" to the next highest cell
				end--;
			}
		}
	}
}

PopData PuyoField::checkPops() {
	PopCounts popCounts{};
	PopColors popColors{};
	PopPositions popPositions{};

	m_bool.reset();

	std::queue<Pos> checkQueue{};
	// Find connected components
	for (int64_t r = m_hrows; r < m_rows; ++r)
	{
		for (int64_t c = 0; c < m_cols; ++c)
		{
			// If the cell has already been checked, or if it's empty, skip
			if (m_bool.get(r, c)) continue;

			Color color = get(r, c);
			// If the cell is empty or a garbage puyo, skip
			if (color <= 1) continue;

			// If the cell is colored, start a group search
			std::vector<Pos> groupPositions{};
			checkQueue.push(Pos{ r, c });

			while (checkQueue.size() > 0)
			{
				Pos current = checkQueue.front();
				if (!m_bool.get(current.r, current.c))
				{
					groupPositions.push_back(current);
					if (current.r > m_hrows && get(current.r - 1, current.c) == color) checkQueue.push(Pos{ current.r - 1, current.c });
					if (current.r < m_rows - 1 && get(current.r + 1, current.c) == color) checkQueue.push(Pos{ current.r + 1, current.c });
					if (current.c > 0 && get(current.r, current.c - 1) == color) checkQueue.push(Pos{ current.r, current.c - 1 });
					if (current.c < m_cols - 1 && get(current.r, current.c + 1) == color) checkQueue.push(Pos{ current.r, current.c + 1 });
				}
				checkQueue.pop();

				// Mark the current cell as checked
				m_bool.set(current.r, current.c, true);
			}

			// Add data to lists if it meets the pop size
			if (static_cast<int64_t>(groupPositions.size()) >= m_puyoToPop) {
				popCounts.push_back(static_cast<int64_t>(groupPositions.size()));
				popColors.insert(color);
				popPositions.insert(popPositions.end(), groupPositions.begin(), groupPositions.end());
			}
		}
	}

	bool hasPops{ static_cast<int64_t>(popCounts.size()) > 0 };

	// Check for adjacent Garbage Puyos and add those to pop positions
	std::vector<Pos> garbagePopPositions{};
	for (Pos& pos : popPositions)
	{
		int64_t r = pos.r;
		int64_t c = pos.c;
		Color garbage = 1;
		if (r > 0 && get(r - 1, c) == garbage) garbagePopPositions.push_back(Pos{ r - 1, c });
		if (r < m_rows - 1 && get(r + 1, c) == garbage) garbagePopPositions.push_back(Pos{ r + 1, c });
		if (c > 0 && get(r, c - 1) == garbage) garbagePopPositions.push_back(Pos{ r, c - 1 });
		if (c < m_cols - 1 && get(r, c + 1) == garbage) garbagePopPositions.push_back(Pos{ r, c + 1 });
	}
	popPositions.insert(popPositions.end(), garbagePopPositions.begin(), garbagePopPositions.end());

	PopData result{ hasPops, popCounts, popColors, popPositions };

	// Remember to reset the boolean matrix though!
	return result;
}

void PuyoField::applyPops(PopPositions& positions)
{
	for (Pos& pos : positions)
	{
		set(pos.r, pos.c, 0);
	}
}

// Simulate the puyo field and return the chain length.
int64_t PuyoField::simulate()
{
	dropPuyos();

	/*std::cout << std::endl;
	std::cout << "Correctly dropped Puyos?" << '\n';*/
	//print();

	auto [hasPops, popCounts, popColors, popPositions] = checkPops();
	//m_bool.reset();

	//std::cout << std::endl;

	if (hasPops) {
		m_chainLength++;
		applyPops(popPositions);
		//std::cout << "Applied pops" << '\n';
		//print();

		simulate();
	}

	return m_chainLength;
}

std::vector<std::tuple<int64_t, int64_t, Color, int64_t>> PuyoField::searchForChains()
{
	removeDrops();
	
	std::vector<std::vector<int64_t>> surface{ surfaceInds() };
	//std::cout << "Surface Inds at: ";
	//for (auto& ind : surface)
	//{
	//	std::cout << ind[0] << "," << ind[1] << " ";
	//}
	//std::cout << '\n';

	std::vector<std::tuple<int64_t, int64_t, Color>> colorAtInds{ tryColorsAtInds(surface) };
	//std::cout << "Colors to try at each index: ";
	//for (auto& rcc : colorAtInds)
	//{
	//	auto [r, c, color] = rcc;
	//	std::cout << "(" << r << ", " << c << ", " << color << "); ";
	//}
	//std::cout << '\n';

	std::vector<PuyoField> tryFields{ fieldsToTry(colorAtInds) };
	//std::cout << "Fields to try: ";
	//for (auto& f : tryFields)
	//{
	//	f.print();
	//	std::cout << "\n";
	//}
	//std::cout << "End of fields to try.\n";

	std::vector<int64_t> chainLengths;
	for (PuyoField f : tryFields)
	{
		int64_t length = f.simulate();
		//std::cout << "Chain Length: " << length << '\n';
		//f.print();
		chainLengths.push_back(length);
	}

	std::vector<std::tuple<int64_t, int64_t, Color, int64_t>> results;
	for (int64_t i = 0; i < static_cast<int64_t>(colorAtInds.size()); i++)
	{
		if (chainLengths[i] >= 2)
		{
			auto [r, c, color] = colorAtInds[i];
			std::tuple<int64_t, int64_t, Color, int64_t> result{ r, c, color, chainLengths[i] };
			results.push_back(result);
		}
	}

	return results;
}

void PuyoField::print() {
	for (int64_t r = 0; r < m_rows; ++r)
	{
		for (int64_t c = 0; c < m_cols; ++c) {
			std::cout << static_cast<int64_t>(get(r, c));
			//std::cout << get(r, c);
		}
		std::cout << '\n';
	}
}

void PuyoField::removeDrops()
{
	for (int64_t c = 0; c < m_cols; c++)
	{
		for (int64_t r = m_rows - 2; r >= 0; r--)
		{
			if (get(r, c) > 0 && get(r + 1, c) == 0)
			{
				set(r, c, 0);
			}
		}
	}
}

std::vector<std::vector<int64_t>> PuyoField::surfaceInds()
{
	std::vector<std::vector<int64_t>> inds;
	for (int64_t c = 0; c < m_cols; c++)
	{
		int64_t zeros{ 0 };
		std::vector<int64_t> ind;
		for (int64_t r = 0; r < m_rows; r++)
		{
			// Count nonzero
			if (get(r, c) == 0) zeros++;
		}

		// Don't add columns that are completely full (-1)
		if (zeros - 1 > -1)
		{
			ind.push_back(zeros - 1);
			ind.push_back(c);
			inds.push_back(ind);
		}
	}

	return inds;
}

std::vector<std::tuple<int64_t, int64_t, Color>> PuyoField::tryColorsAtInds(std::vector<std::vector<int64_t>>& inds)
{
	std::vector<std::tuple<int64_t, int64_t, Color>> result;

	for (auto& ind : inds)
	{
		int64_t r = ind[0];
		int64_t c = ind[1];
		if (c > 0 && get(r, c - 1) > 1)
		{
			std::tuple<int64_t, int64_t, Color> toTry{ r, c, get(r, c - 1) };
			result.push_back(toTry);
		}
		if (c < m_cols - 1 && get(r, c + 1) > 1)
		{
			std::tuple<int64_t, int64_t, Color> toTry{ r, c, get(r, c + 1) };
			result.push_back(toTry);
		}
		if (r < m_rows - 1 && get(r + 1, c) > 1)
		{
			std::tuple<int64_t, int64_t, Color> toTry{ r, c, get(r + 1, c) };
			result.push_back(toTry);
		}
	}

	return result;
}

std::vector<PuyoField> PuyoField::fieldsToTry(std::vector<std::tuple<int64_t, int64_t, Color>>& colorsAtInds)
{
	std::vector<PuyoField> fields;
	for (auto& rcCol : colorsAtInds)
	{
		auto [r, c, color] = rcCol;
		//PuyoField(int64_t rows, int64_t cols, int64_t hrows, int64_t puyoToPop)
		PuyoField testField{ m_rows, m_cols, m_hrows, m_puyoToPop };
		copyTo(testField);

		testField.set(r, c, color);

		// Try to add a second Puyo above to ensure a pop
		if (r > 1)
		{
			testField.set(r - 1, c, color);
		}

		fields.push_back(testField);
	}
	return fields;
}

