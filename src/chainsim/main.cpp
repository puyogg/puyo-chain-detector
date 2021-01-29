#include <iostream>
#include <vector>
#include "Fields.hpp"

int main()
{
    Chainsim::PuyoField field;
    std::vector<std::vector<int>> test{
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
    field.setIntField(test);
    field.print();
    std::cout << "Finished!\n";

    auto search = field.searchForChains();
    std::cout << "Found " << search.size() << " chains to try.\n";

    for (auto& s : search)
    {
        auto& [pos, color, length] = s;
        std::cout << pos.r << ", " << pos.c << ", " << static_cast<int>(color) << ", " << length << ";\n";
    }

    return 0;
}
