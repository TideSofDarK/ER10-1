#pragma once

#include <array>
#include <bitset>
#include "Math.hxx"
#include "Tile.hxx"
#include "SharedConstants.hxx"

struct SDrawDoorInfo
{
    SVec2Int TileCoords{};
    SDirection Direction{};
    STimeline Timeline{ 2.0f };

    SDrawDoorInfo()
    {
        Invalidate();
    }

    void Set(SVec2Int NewTileCoords, SDirection NewDirection)
    {
        TileCoords = NewTileCoords;
        Direction = NewDirection;
        Timeline.Reset();
    }

    void Invalidate()
    {
        TileCoords = { -1, -1 };
    }
};

namespace ELevelDirtyFlags
{
    using Type = uint32_t;
    enum : Type
    {
        POVChanged = 1 << 0,
        DrawSet = 1 << 2,
        DirtyRange = 1 << 3,
        All = UINT32_MAX
    };
}

struct STilemap
{
    int32_t Width{};
    int32_t Height{};
    std::array<STile, MAX_LEVEL_TILE_COUNT> Tiles{};
    std::bitset<(MAX_LEVEL_WIDTH + 1) * (MAX_LEVEL_HEIGHT + 1)> WallJoints{};
    uint32_t bUseWallJoints = true;

    [[nodiscard]] uint32_t TileCount() const { return Width * Height; }

    [[nodiscard]] STile* GetTileAtMutable(const SVec2Int& Coords)
    {
        if (IsValidTile(Coords))
        {
            auto Index = CoordsToIndex(Coords);
            return &Tiles[Index];
        }
        return nullptr;
    }

    [[nodiscard]] STile* GetNeighborTileAtMutable(const SVec2Int& Coords, SDirection Direction)
    {
        SVec2Int NeighborCoords = Direction.GetVector<int>() + Coords;
        return GetTileAtMutable(NeighborCoords);
    }

    void SetWallJoint(const SVec2Int& Coords, bool bValue = true)
    {
        if (IsValidWallJoint(Coords))
        {
            auto Index = WallJointCoordsToIndex(Coords.X, Coords.Y);
            WallJoints.set(Index, bValue);
        }
    }

    [[nodiscard]] STile const* GetTileAt(const SVec2Int& Coords) const
    {
        if (IsValidTile(Coords))
        {
            auto Index = CoordsToIndex(Coords.X, Coords.Y);
            return &Tiles[Index];
        }
        return nullptr;
    }

    [[nodiscard]] STile const* GetTile(const std::size_t Index) const
    {
        if (Index < MAX_LEVEL_TILE_COUNT)
        {
            return &Tiles[Index];
        }
        return nullptr;
    }

    [[nodiscard]] bool IsValidTile(const SVec2Int& Coords) const
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

    [[nodiscard]] inline std::size_t CoordsToIndex(int X, int Y) const { return (Y * Width) + X; }

    [[nodiscard]] inline std::size_t CoordsToIndex(const SVec2Int& Coords) const { return CoordsToIndex(Coords.X, Coords.Y); }

    [[nodiscard]] bool IsWallJointAt(SVec2Int Coords) const
    {
        auto Index = WallJointCoordsToIndex(Coords.X, Coords.Y);
        return WallJoints.test(Index);
    }

    [[nodiscard]] bool IsValidWallJoint(SVec2Int Coords) const
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

    [[nodiscard]] inline std::size_t WallJointCoordsToIndex(int X, int Y) const { return (Y * (Width + 1)) + X; }

    [[nodiscard]] SVec2Int CalculateMapSize() const
    {
        return {
            MAP_TILE_SIZE_PIXELS * Width + MAP_TILE_EDGE_SIZE_PIXELS,
            MAP_TILE_SIZE_PIXELS * Height + MAP_TILE_EDGE_SIZE_PIXELS
        };
    }

    [[nodiscard]] SVec2Int CalculateMapIsoSize() const
    {
        return {
            MAP_ISO_TILE_SIZE_PIXELS * Width + MAP_ISO_TILE_EDGE_SIZE_PIXELS,
            MAP_ISO_TILE_SIZE_PIXELS * Height + MAP_ISO_TILE_EDGE_SIZE_PIXELS
        };
    }

    void PostProcess();

    void ToggleEdge(const SVec2Int& Coords, SDirection Direction, UFlagType NorthEdgeBit);

    void Edit(const SVec2Int& Coords, ETileFlag Flag, bool bHandleEdges = true);

    void EditBlock(const SRectInt& Rect, ETileFlag Flag);

    void Serialize(std::ofstream& Stream) const;

    void Deserialize(std::istream& Stream);
};

