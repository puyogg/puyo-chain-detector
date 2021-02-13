#include "Fields.hpp"
#include <iostream>

namespace Chainsim {

// Field template class
template <class T>
Field<T>::Field() : m_rows{ DEFAULT_ROWS }, m_cols{ DEFAULT_COLS }
{
}

template <class T>
Field<T>::Field(int rows, int cols) : m_rows{ rows }, m_cols{ cols }
{
}

template <class T>
T Field<T>::get(int row, int col)
{
    return m_data[row * m_cols + col];
}

template <class T>
void Field<T>::set(int row, int col, T value)
{
    m_data[row * m_cols + col] = value;
}

template <class T>
void Field<T>::set(std::vector<T>& vector1d)
{
    for (int r = 0; r < m_rows; r++)
    {
        for (int c = 0; c < m_cols; c++)
        {
            set(r, c, vector1d[r * m_cols + c]);
        }
    }
}

template <class T>
void Field<T>::set(std::vector<std::vector<T>>& vector2d)
{
    int rows = static_cast<int>(vector2d.size());
    int cols = static_cast<int>(vector2d[0].size());

    for (int r = 0; r < rows; r++)
    {
        for (int c = 0; c < cols; c++)
        {
            set(r, c, vector2d[r][c]);
        }
    }
}

template <class T>
void Field<T>::print()
{
    for (int r = 0; r < m_rows; r++)
    {
        for (int c = 0; c < m_cols; c++)
        {
            std::cout << static_cast<int>(get(r, c));
        }
        std::cout << '\n';
    }
}

template <class T>
void Field<T>::reset()
{
}

template <class T>
void Field<T>::copyTo(Field<T>& field)
{
    for (int r = 0; r < m_rows; r++)
    {
        for (int c = 0; c < m_cols; c++)
        {
            field.set(r, c, get(r, c));
        }
    }
}

// Explicit template instantiation
template class Field<bool>;
template class Field<int>;
template class Field<Color>;


}
