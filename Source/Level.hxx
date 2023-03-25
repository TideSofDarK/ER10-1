#pragma once

#include <array>
#include "Tile.hxx"

int constexpr LevelGridSize = 16 * 16;

struct SLevel {
    int Width{};
    int Height{};
    std::array<STile, LevelGridSize> Grid{};

    STile const & GetTileAt(int X, int Y);
    [[nodiscard]] inline int CoordsToIndex(int X, int Y) const { return (Y * Width) + X; };
};


