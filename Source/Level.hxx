#pragma once

#include <array>
#include <optional>
#include "Tile.hxx"

#define MAX_LEVEL_WIDTH 32
#define MAX_LEVEL_HEIGHT 32
#define MAX_LEVEL_TILE_COUNT (MAX_LEVEL_WIDTH * MAX_LEVEL_HEIGHT)

struct SLevel
{
    using UWallJoint = bool;

    int Width{};
    int Height{};
    std::array<STile, MAX_LEVEL_TILE_COUNT> Tiles{};
    std::array<UWallJoint, (MAX_LEVEL_WIDTH + 1) * (MAX_LEVEL_HEIGHT + 1)> WallJoints{};
    bool bUseWallJoints = false;

    [[nodiscard]] STile* GetTileAtMutable(UVec2Int Coords)
    {
        if (IsValidTile(Coords))
        {
            auto Index = CoordsToIndex(Coords.X, Coords.Y);
            return &Tiles[Index];
        }
        return nullptr;
    }

    [[nodiscard]] UWallJoint* GetWallJointAtMutable(UVec2Int Coords)
    {
        if (IsValidWallJoint(Coords))
        {
            auto Index = WallJointCoordsToIndex(Coords.X, Coords.Y);
            return &WallJoints[Index];
        }
        return nullptr;
    }

    [[nodiscard]] STile const* GetTileAt(UVec2Int Coords) const
    {
        if (IsValidTile(Coords))
        {
            auto Index = CoordsToIndex(Coords.X, Coords.Y);
            return &Tiles[Index];
        }
        return nullptr;
    }

    [[nodiscard]] bool IsValidTile(UVec2Int Coords) const
    {
        return IsValidTileX(Coords.X) && IsValidTileY(Coords.Y);
    }

    [[nodiscard]] bool IsValidTileX(int Coord) const
    {
        return Coord >= 0 && Coord < Width;
    }

    [[nodiscard]] bool IsValidTileY(int Coord) const
    {
        return Coord >= 0 && Coord < Height;
    }

    [[nodiscard]] inline int CoordsToIndex(int X, int Y) const { return (Y * Width) + X; }

    [[nodiscard]] UWallJoint GetWallJointAt(UVec2Int Coords) const
    {
        auto Index = WallJointCoordsToIndex(Coords.X, Coords.Y);
        return WallJoints[Index];
    }

    [[nodiscard]] bool IsValidWallJoint(UVec2Int Coords) const
    {
        return IsValidWallJointX(Coords.X) && IsValidWallJointY(Coords.Y);
    }

    [[nodiscard]] bool IsValidWallJointX(int Coord) const
    {
        return Coord >= 0 && Coord < Width + 1;
    }

    [[nodiscard]] bool IsValidWallJointY(int Coord) const
    {
        return Coord >= 0 && Coord < Height + 1;
    }

    [[nodiscard]] inline int WallJointCoordsToIndex(int X, int Y) const { return (Y * (Width + 1)) + X; }

    void InitWallJoints();

    void Excavate(UVec2Int Coords);
};
