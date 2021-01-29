#pragma once
#include "Constants.hpp"
#include <vector>

namespace Chainsim {


template <class T> class Field
{
public:
    std::vector<T> m_data;
    int m_rows;
    int m_cols;
    Field();
    Field(int rows, int cols);
    T get(int row, int col);
    void set(int row, int col, T value);
    void set(std::vector<T>& vector1d);
    void set(std::vector<std::vector<T>>& vector2d);
    void copyTo(Field<T>& field);
    void print();
    virtual void reset();
};

class BoolField : public Field<bool>
{
private:
    using Field::m_rows;
    using Field::m_cols;
public:
    BoolField();
    BoolField(int rows, int cols);

    void reset();
};

class PuyoField : public Field<Color>
{
private:
    using Field::m_rows;
    using Field::m_cols;
    BoolField m_bool;
    int m_hrows;
    int m_puyoToPop;
    int m_chainLength;

    /**
     * Helper function to check if a Puyo is colored (not empty or garbage)
     * @return
     */
    bool isColoredAt(int row, int col);

    /**
     * Apply a gravity step to the PuyoField
     */
    void dropPuyos();

    /**
     * Set field cells to Color::NONE based on the PopPositions data
     */
    void applyPops(PopPositions& positions);

    /**
     * Ignore Puyos that are floating. Useful if you want to ignore
     * them during realtime analysis.
     */
    void removeFloaters();

    /**
     * Find the surface locations of the current field.
     * @return ...
     */
    std::vector<Pos> surfacePositions();

    std::vector<std::tuple<Pos, Color>> tryColorsAtPositions(std::vector<Pos>& positions);

    /**
     * Generate all the PuyoFields to try
     * @param colorsAtInds
     * @return
     */
    std::vector<PuyoField> fieldsToTry(std::vector<std::tuple<Pos, Color>>& colorsAtPositions);

public:
    PuyoField();
    PuyoField(int rows, int cols);
    PuyoField(int rows, int cols, int hrows);
    PuyoField(int rows, int cols, int hrows, int puyoToPop);

    void reset();

    /**
     * Additional setter to cast intFields to Color enum
     * @param intField
     */
    void setIntField(std::vector<std::vector<int>> intVec2d);

    /**
     * Helper function to check for colored groups of 4 or more,
     * and also for any garbage Puyos that need to be removed.
     * @return tuple of <HasPops, PopCounts, PopColors, PopPositions>
     */
    PopData checkPops();

    /**
     * Simulate the PuyoField by running every chain step to completion.
     * This method modifies the original Field.
     * @return Length of the chain
     */
    int simulate();

    std::vector<std::tuple<Pos, Color, int>> searchForChains();
};

}
