#include "Fields.hpp"
#include <iostream>

namespace Chainsim
{

// BoolField class
BoolField::BoolField() : Field<bool>()
{
    m_data = std::vector<bool>(m_rows * m_cols, false);
}

BoolField::BoolField(int rows, int cols) : Field<bool>(rows, cols)
{
    m_data = std::vector<bool>(m_rows * m_cols, false);
}


void BoolField::reset()
{
    std::fill(m_data.begin(), m_data.end(), false);
}


}
