#include "Fields.hpp"
#include <algorithm>

namespace Chainsim
{

// BoolField class
IntField::IntField() : Field<int>()
{
    m_data = std::vector<int>(m_rows * m_cols, 0);
}

IntField::IntField(int rows, int cols) : Field<int>(rows, cols)
{
    m_data = std::vector<int>(m_rows * m_cols, 0);
}


void IntField::reset()
{
    std::fill(m_data.begin(), m_data.end(), 0);
}


}
