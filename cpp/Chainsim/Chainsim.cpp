// Chainsim.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <vector>
#include "PuyoField.h"

int main()
{
    Chainsim::PuyoField field;
    std::vector<std::vector<Chainsim::Color>> test{
        {0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0},
        {0, 4, 0, 0, 0, 0},
        {0, 4, 0, 0, 0, 0},
        {3, 6, 4, 0, 6, 0},
        {6, 6, 4, 0, 3, 3},
        {3, 3, 6, 0, 6, 6},
        {4, 4, 4, 3, 6, 3},
        {6, 3, 3, 4, 4, 2},
        {4, 4, 2, 6, 6, 2},
    };
    std::cout << '\n';
    field.set(test);
    field.print();
    std::cout << "\n";

    std::vector<std::tuple<int64_t, int64_t, Chainsim::Color, int64_t>> search = field.searchForChains();
    std::cout << "Found " << search.size() << " chains to try.\n";
    for (auto& s : search)
    {
        auto [r, c, color, length] = s;
        std::cout << r << ", " << c << ", " << color << ", " << length << ";\n";
    }

    return 0;
}
