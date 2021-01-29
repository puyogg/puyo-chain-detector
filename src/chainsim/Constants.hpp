#pragma once
#include <cstddef>
#include <vector>
#include <set>
#include <tuple>

namespace Chainsim
{

constexpr std::size_t DEFAULT_COLS{6};
constexpr std::size_t DEFAULT_ROWS{13};
constexpr std::size_t DEFAULT_HROWS{1};
constexpr int DEFAULT_PUYOTOPOP{4};

struct Pos
{
    int r;
    int c;
};

enum class Color
{
    NONE,
    GARBAGE,
    RED,
    GREEN,
    BLUE,
    YELLOW,
    PURPLE
};

using PopCounts = std::vector<int>;
using PopColors = std::set<Color>;
using PopPositions = std::vector<Pos>;
using HasPop = bool;
using PopData = std::tuple<HasPop, PopCounts, PopColors, PopPositions>;

}
