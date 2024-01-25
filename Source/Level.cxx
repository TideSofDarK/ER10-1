#include "Level.hxx"

#include <iostream>
#include <fstream>
#include "CommonTypes.hxx"
#include "Tile.hxx"

void STilemap::PostProcess()
{
    WallJoints.reset();
    if (!bUseWallJoints)
    {
        return;
    }
    UVec2Int Coords{};
    for (; Coords.X < Width; ++Coords.X)
    {
        for (Coords.Y = 0; Coords.Y < Height; ++Coords.Y)
        {
            auto CurrentTile = GetTileAtMutable(Coords);
            if (CurrentTile == nullptr)
            {
                continue;
            }
            if (CurrentTile->IsWallBasedEdge(SDirection::North()) && CurrentTile->IsWallBasedEdge(SDirection::West()))
            {
                SetWallJoint(Coords);
            }
            if (CurrentTile->IsWallBasedEdge(SDirection::North()) && CurrentTile->IsWallBasedEdge(SDirection::East()))
            {
                SetWallJoint({ Coords.X + 1, Coords.Y });
            }
            if (CurrentTile->IsWallBasedEdge(SDirection::South()) && CurrentTile->IsWallBasedEdge(SDirection::East()))
            {
                SetWallJoint({ Coords.X + 1, Coords.Y + 1 });
            }
            if (CurrentTile->IsWallBasedEdge(SDirection::South()) && CurrentTile->IsWallBasedEdge(SDirection::West()))
            {
                SetWallJoint({ Coords.X, Coords.Y + 1 });
            }
        }
    }
}

void STilemap::ToggleEdge(const UVec2Int& Coords, SDirection Direction, uint32_t NorthEdgeBit)
{
    auto Tile = GetTileAtMutable(Coords);
    if (Tile == nullptr)
    {
        return;
    }

    auto& EdgeFlags = Tile->EdgeFlags;
    EdgeFlags = EdgeFlags ^ STile::DirectionBit(NorthEdgeBit, Direction);

    auto NeighborTile = GetNeighborTileAtMutable(Coords, Direction);
    if (NeighborTile == nullptr)
    {
        return;
    }

    auto& NeighborEdgeFlags = NeighborTile->EdgeFlags;
    NeighborEdgeFlags = NeighborEdgeFlags ^ STile::DirectionBit(NorthEdgeBit, Direction.Inverted());
}

void STilemap::Excavate(UVec2Int Coords)
{
    if (!IsValidTile(Coords))
    {
        return;
    }
    auto Tile = GetTileAtMutable(Coords);
    Tile->Flags |= TILE_FLOOR_BIT;

    for (SDirection::Type Direction = 0; Direction < SDirection::Count; ++Direction)
    {
        auto& EdgeFlags = Tile->EdgeFlags;

        auto NeighborTile = GetTileAtMutable(Coords + SDirection{ Direction }.GetVector<int>());
        if (NeighborTile != nullptr)
        {
            auto NeighborDirection = SDirection{ Direction }.Inverted();

            if (NeighborTile->CheckFlag(TILE_FLOOR_BIT))
            {
                Tile->ClearEdgeFlags(SDirection{ Direction });
                NeighborTile->ClearEdgeFlags(NeighborDirection);
            }
            else
            {
                Tile->SetWall(SDirection{ Direction });
                NeighborTile->SetWall(NeighborDirection);
            }
        }
        else
        {

            Tile->SetWall(SDirection{ Direction });
        }
    }
}

void STilemap::ExcavateBlock(const URectInt& Rect)
{
    if (Rect.Min == Rect.Max)
    {
        Excavate(Rect.Max);
    }

    for (auto X = Rect.Min.X; X <= Rect.Max.X; ++X)
    {
        for (auto Y = Rect.Min.Y; Y <= Rect.Max.Y; ++Y)
        {
            Excavate({ X, Y });
        }
    }
}

void STilemap::Cover(UVec2Int Coords)
{
    // if (!IsValidTile(Coords))
    // {
    //     return;
    // }
    // auto Tile = GetTileAtMutable(Coords);
    // Tile->Type = ETileType::Empty;
    //
    // for (SDirection::Type Direction = 0; Direction < SDirection::Count; ++Direction)
    // {
    //     auto& TileEdge = Tile->Edges[Direction];
    //
    //     auto NeighborTile = GetTileAtMutable(Coords + SDirection{ Direction }.GetVector<int>());
    //     if (NeighborTile != nullptr)
    //     {
    //         auto NeighborDirection = SDirection{ Direction }.Inverted();
    //
    //         if (NeighborTile->Type == ETileType::Floor)
    //         {
    //             TileEdge = ETileEdgeType::Wall;
    //
    //             NeighborTile->Edges[NeighborDirection.Index] = ETileEdgeType::Wall;
    //         }
    //         else
    //         {
    //             TileEdge = ETileEdgeType::Empty;
    //
    //             NeighborTile->Edges[NeighborDirection.Index] = ETileEdgeType::Empty;
    //         }
    //     }
    //     else
    //     {
    //         TileEdge = ETileEdgeType::Wall;
    //     }
    // }
}

void STilemap::Serialize(std::ofstream& Stream) const
{
    Serialization::Write32(Stream, Width);
    Serialization::Write32(Stream, Height);

    for (auto& Tile : Tiles)
    {
        Tile.Serialize(Stream);
    }

    Serialization::Write32(Stream, bUseWallJoints);
}

void STilemap::Deserialize(std::istream& Stream)
{
    Serialization::Read32(Stream, Width);
    Serialization::Read32(Stream, Height);

    for (auto& Tile : Tiles)
    {
        Tile.Deserialize(Stream);
    }

    Serialization::Read32(Stream, bUseWallJoints);

    PostProcess();
}

void SLevel::Update(float DeltaTime)
{
    DrawState.DoorInfo.Timeline.Advance(DeltaTime);
}

void SLevel::MarkDirty()
{
    DrawState.bDirty = true;
}
