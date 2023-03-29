#pragma once

#include <array>
#include "Tile.hxx"

int constexpr LevelGridSize = 16 * 16;

struct SLevel {
    int Width{};
    int Height{};
    std::array<STile, LevelGridSize> Grid{};
    bool bUseWallJoints = true;

    STile const &GetTileAt(int X, int Y);

    [[nodiscard]] bool IsValidTile(UVec2Int Coords) const {
        return IsValidCoordX(Coords.X) && IsValidCoordY(Coords.Y);
    }

    [[nodiscard]] bool IsValidCoordX(int Coord) const {
        return Coord >= 0 && Coord < Width;
    }

    [[nodiscard]] bool IsValidCoordY(int Coord) const {
        return Coord >= 0 && Coord < Height;
    }

    [[nodiscard]] inline int CoordsToIndex(int X, int Y) const { return (Y * Width) + X; };
};


