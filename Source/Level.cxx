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
    for (; Coords.X < (int)Width; ++Coords.X)
    {
        for (Coords.Y = 0; Coords.Y < (int)Height; ++Coords.Y)
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

void STilemap::ToggleEdge(const UVec2Int& Coords, SDirection Direction, UFlagType NorthEdgeBit)
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

void STilemap::Edit(const UVec2Int& Coords, ETileFlag Flag, bool bHandleEdges)
{
    if (!IsValidTile(Coords))
    {
        return;
    }
    auto Tile = GetTileAtMutable(Coords);
    Tile->Flags = Flag;

    if (!bHandleEdges)
    {
        return;
    }

    for (auto& Direction : SDirection::All())
    {
        auto NeighborTile = GetTileAtMutable(Coords + Direction.GetVector<int>());
        if (NeighborTile != nullptr)
        {
            auto NeighborDirection = Direction.Inverted();

            if (NeighborTile->CheckFlag(Flag) || (NeighborTile->Flags == 0 && Flag == 0) || (NeighborTile->Flags != 0 && Flag != 0))
            {
                Tile->ClearEdgeFlags(Direction);
                NeighborTile->ClearEdgeFlags(NeighborDirection);
            }
            else
            {
                Tile->SetWall(Direction);
                NeighborTile->SetWall(NeighborDirection);
            }
        }
        else
        {
            Tile->SetWall(Direction);
        }
    }
}

void STilemap::EditBlock(const URectInt& Rect, ETileFlag Flag)
{
    if (Rect.Min == Rect.Max)
    {
        Edit(Rect.Max, Flag);
    }

    for (auto X = Rect.Min.X; X <= Rect.Max.X; ++X)
    {
        for (auto Y = Rect.Min.Y; Y <= Rect.Max.Y; ++Y)
        {
            Edit({ X, Y }, Flag);
        }
    }
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
    DoorInfo.Timeline.Advance(DeltaTime);
}
