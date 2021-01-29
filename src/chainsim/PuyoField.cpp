#include "Fields.hpp"
#include <queue>
#include <iostream>

namespace Chainsim {

// Puyo Field class
PuyoField::PuyoField()
    : Field<Color>(DEFAULT_ROWS, DEFAULT_COLS),
      m_hrows{ DEFAULT_HROWS },
      m_puyoToPop{ DEFAULT_PUYOTOPOP }
{
    m_data = std::vector<Color>(m_cols * m_rows, Color::NONE);
    m_bool = BoolField(m_rows, m_cols);
    m_chainLength = 0;
}

PuyoField::PuyoField(int rows, int cols)
    : Field<Color>(rows, cols),
      m_hrows{ DEFAULT_HROWS },
      m_puyoToPop{ DEFAULT_PUYOTOPOP}
{
    m_data = std::vector<Color>(m_cols * m_rows, Color::NONE);
    m_chainLength = 0;
}

PuyoField::PuyoField(int rows, int cols, int hrows)
    : Field<Color>(rows, cols),
      m_hrows{ hrows },
      m_puyoToPop{ DEFAULT_PUYOTOPOP}
{
    m_data = std::vector<Color>(m_cols * m_rows, Color::NONE);
    m_chainLength = 0;
}

PuyoField::PuyoField(int rows, int cols, int hrows, int puyoToPop)
    : Field<Color>(rows, cols),
      m_hrows{ hrows },
      m_puyoToPop{ puyoToPop }
{
    m_data = std::vector<Color>(m_cols * m_rows, Color::NONE);
    m_chainLength = 0;
}

void PuyoField::setIntField(std::vector<std::vector<int>> intVec2d)
{
    int rows { static_cast<int>(intVec2d.size()) };
    int cols { static_cast<int>(intVec2d.at(0).size()) };

    for (int r = 0; r < rows; r++)
    {
        for (int c = 0; c < cols; c++)
        {
            Field<Color>::set(r, c, static_cast<Color>(intVec2d[r][c]));
        }
    }
}

void PuyoField::reset()
{
    std::fill(m_data.begin(), m_data.end(), Color::NONE);
}

void PuyoField::dropPuyos()
{
    // In each column...
    for (int c = 0; c < m_cols; c++)
    {
        int end = m_rows - 1;

        // Traverse the rows bottom to top
        for (int r = end; r >= 0; r--)
        {
            Color current = get(r, c);

            // If the current cell isn't empty...
            if (get(r, c) != Color::NONE)
            {
                // Move it to the cell currently marked as "end"
                set(end, c, current);
                // Empty the current cell
                if (r != end) set(r, c, Color::NONE);
                // Shift "end" to the next highest cell
                end--;
            }
        }
    }
}

bool PuyoField::isColoredAt(int row, int col)
{
    Color cellColor{ get(row, col) };
    return cellColor >= Color::RED && cellColor <= Color::PURPLE;
}

PopData PuyoField::checkPops()
{
    PopCounts popCounts;
    PopColors popColors;
    PopPositions popPositions;

    m_bool.reset();

    std::queue<Pos> checkQueue{};

    // Find connected components
    for (int r = m_hrows; r < m_rows; r++)
    {
        for (int c = 0; c < m_cols; c++)
        {
            // If the cell has already been checked, or if it's empty, skip
            if (m_bool.get(r, c)) continue;

            Color color = get(r, c);
            // If the cell is empty or a garbage Puyo, skip
            if (color == Color::NONE || color == Color::GARBAGE) continue;

            // Otherwise, the cell is colored. Start a group search, starting
            // at the current position.
            std::vector<Pos> groupPositions{};
            checkQueue.push(Pos{r, c});

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
            if (static_cast<int>(groupPositions.size()) >= m_puyoToPop)
            {
                popCounts.push_back(static_cast<int>(groupPositions.size()));
                popColors.insert(color);
                popPositions.insert(popPositions.end(), groupPositions.begin(), groupPositions.end());
            }
        }
    }

    bool hasPops{ static_cast<int>(popCounts.size()) > 0 };

    // Check for adjacent Garbage Puyos and add those to pop positions
    std::vector<Pos> garbagePopPositions{};
    for (Pos& pos : popPositions)
    {
        int r = pos.r;
        int c = pos.c;

        if (r > 0 && get(r - 1, c) == Color::GARBAGE) garbagePopPositions.push_back(Pos{ r - 1, c });
        if (r < m_rows - 1 && get(r + 1, c) == Color::GARBAGE) garbagePopPositions.push_back(Pos{ r + 1, c });
        if (c > 0 && get(r, c - 1) == Color::GARBAGE) garbagePopPositions.push_back(Pos{ r, c - 1 });
        if (c < m_cols - 1 && get(r, c + 1) == Color::GARBAGE) garbagePopPositions.push_back(Pos{ r, c + 1 });
    }
    popPositions.insert(popPositions.end(), garbagePopPositions.begin(), garbagePopPositions.end());

    PopData result{ hasPops, popCounts, popColors, popPositions };

    return result;
}

void PuyoField::applyPops(PopPositions& positions)
{
    for (Pos& pos: positions)
    {
        set(pos.r, pos.c, Color::NONE);
    }
}

int PuyoField::simulate()
{
    dropPuyos();

    auto [hasPops, popCounts, popColors, popPositions] = checkPops();

    if (hasPops)
    {
        m_chainLength++;
        applyPops(popPositions);
        simulate();
    }

    return m_chainLength;
}

void PuyoField::removeFloaters()
{
    for (int64_t c = 0; c < m_cols; c++)
    {
        for (int64_t r = m_rows - 2; r >= 0; r--)
        {
            bool isPuyo = get(r, c) != Color::NONE;
            bool belowIsNone = get(r + 1, c) == Color::NONE;
            if (isPuyo && belowIsNone)
            {
                set(r, c, Color::NONE);
            }
        }
    }
}

std::vector<Pos> PuyoField::surfacePositions()
{
    // vector nesting: columns -> row
    std::vector<Pos> inds;
    for (int c = 0; c < m_cols; c++)
    {
        // Count the number of empty cells in a column
        int nones { -1 };
        for (int r = 0; r < m_rows; r++)
        {
            if (get(r, c) == Color::NONE) nones++;
        }

        // Don't add columns that are completely full (-1)
        if (nones > -1)
        {
            Pos pos{ nones, c };
            inds.push_back(pos);
        }
    }

    return inds;
}

std::vector<std::tuple<Pos, Color>> PuyoField::tryColorsAtPositions(std::vector<Pos>& positions)
{
    std::vector<std::tuple<Pos, Color>> result;

    for (Pos& pos: positions)
    {
        int r = pos.r;
        int c = pos.c;

        // Try out colors based on the surrounding colors
        // Look left
        if (c > 0 && isColoredAt(r, c - 1))
        {
            Pos tryPos{ r, c };
            Color tryColor{ get(r, c - 1 ) };
            result.push_back(std::tuple(tryPos, tryColor));
        }

        // Look right
        if (c < m_cols - 1 && isColoredAt(r, c + 1))
        {
            Pos tryPos{ r, c };
            Color tryColor{ get(r, c + 1) };
            result.push_back(std::tuple(tryPos, tryColor));
        }

        // Look down
        if (r < m_rows - 1 && isColoredAt(r + 1, c))
        {
            Pos tryPos{ r, c };
            Color tryColor{ get(r + 1, c) };
            result.push_back(std::tuple(tryPos, tryColor));
        }
    }

    return result;
}

std::vector<PuyoField> PuyoField::fieldsToTry(std::vector<std::tuple<Pos, Color>>& colorsAtPositions)
{
    std::vector<PuyoField> fields;
    for (auto& posCol : colorsAtPositions)
    {
        auto [pos, color] = posCol;
        PuyoField testField(m_rows, m_cols, m_hrows, m_puyoToPop);
        copyTo(testField);
        testField.set(pos.r, pos.c, color);

        // Try to add a second Puyo above to ensure a pop
        if (pos.r > 1)
        {
            testField.set(pos.r - 1, pos.c, color);
        }

        fields.push_back(testField);
    }

    return fields;
}

std::vector<std::tuple<Pos, Color, int>> PuyoField::searchForChains()
{
    // Ignore floating Puyos
    removeFloaters();

    // Calculate surface locations
    std::vector<Pos> positions{ surfacePositions() };

    // Use those surface locations to find what colors to try at them
    std::vector<std::tuple<Pos, Color>> colorsAtPositions{ tryColorsAtPositions(positions) };

    // Use the color+pos combination to generate fields.
    // Vector should be the same length as colorsAtPositions
    std::vector<PuyoField> tryFields{ fieldsToTry(colorsAtPositions) };

    // Get the length of each chain
    std::vector<std::tuple<Pos, Color, int>> results;
    for (int i = 0; i < static_cast<int>(tryFields.size()); i++)
    {
        int length{ tryFields.at(i).simulate() };
        if (length >= 2)
        {
            auto [pos, color] = colorsAtPositions.at(i);
            std::tuple<Pos, Color, int> result{ pos, color, length };
            results.push_back(result);
        }
    }

    return results;
}

} // end Chainsim namespace
