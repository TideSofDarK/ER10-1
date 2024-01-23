#pragma once

#include <array>
#include <bitset>
#include "Math.hxx"
#include "Tile.hxx"
#include "SharedConstants.hxx"

struct SDrawDoorInfo
{
    UVec2Int TileCoords{};
    SDirection Direction{};
    STimeline Timeline{ 0.0f, 2.0f };

    SDrawDoorInfo()
    {
        Invalidate();
    }

    void Set(UVec2Int NewTileCoords, SDirection NewDirection)
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

struct SDrawLevelState
{
    SDrawDoorInfo DoorInfo;
    bool bDirty = true;
};

struct STilemap
{
    uint32_t Width{};
    uint32_t Height{};
    std::array<STile, MAX_LEVEL_TILE_COUNT> Tiles{};
    std::bitset<(MAX_LEVEL_WIDTH + 1) * (MAX_LEVEL_HEIGHT + 1)> WallJoints{};
    uint32_t bUseWallJoints = true;

    [[nodiscard]] STile* GetTileAtMutable(const UVec2Int& Coords)
    {
        if (IsValidTile(Coords))
        {
            auto Index = CoordsToIndex(Coords.X, Coords.Y);
            return &Tiles[Index];
        }
        return nullptr;
    }

    [[nodiscard]] STile* GetNeighborTileAtMutable(const UVec2Int& Coords, SDirection Direction)
    {
        UVec2Int NeighborCoords = Direction.GetVector<int>() + Coords;
        return GetTileAtMutable(NeighborCoords);
    }

    void SetWallJoint(const UVec2Int& Coords, bool bValue = true)
    {
        if (IsValidWallJoint(Coords))
        {
            auto Index = WallJointCoordsToIndex(Coords.X, Coords.Y);
            WallJoints.set(Index, bValue);
        }
    }

    [[nodiscard]] STile const* GetTileAt(const UVec2Int& Coords) const
    {
        if (IsValidTile(Coords))
        {
            auto Index = CoordsToIndex(Coords.X, Coords.Y);
            return &Tiles[Index];
        }
        return nullptr;
    }

    [[nodiscard]] bool IsValidTile(const UVec2Int& Coords) const
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

    [[nodiscard]] bool IsWallJointAt(UVec2Int Coords) const
    {
        auto Index = WallJointCoordsToIndex(Coords.X, Coords.Y);
        return WallJoints.test(Index);
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

    [[nodiscard]] inline std::size_t WallJointCoordsToIndex(int X, int Y) const { return (Y * (Width + 1)) + X; }

    void PostProcess();

    void ToggleEdge(const UVec2Int& Coords, SDirection Direction, ETileEdgeType TileEdgeType);

    void Excavate(UVec2Int Coords);

    void ExcavateBlock(const URectInt& Rect);

    void Cover(UVec2Int Coords);

    void Serialize(std::ofstream& Stream) const;

    void Deserialize(std::ifstream& Stream);
};

struct SLevel : STilemap
{
    int Z{};
    SDrawLevelState DrawState;

    void Update(float DeltaTime);

    void MarkDirty();
};
