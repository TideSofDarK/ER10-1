#include "Level.hxx"

#include "Tile.hxx"

void STilemap::InitWallJoints()
{
    WallJoints.reset();
    bUseWallJoints = true;
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

void STilemap::ToggleEdge(const UVec2Int& Coords, SDirection Direction, ETileEdgeType TileEdgeType)
{
    auto Tile = GetTileAtMutable(Coords);
    if (Tile == nullptr)
    {
        return;
    }

    auto& TileEdge = Tile->Edges[Direction.Index];
    TileEdge = TileEdge == TileEdgeType ? ETileEdgeType::Wall : TileEdgeType;

    auto NeighborTile = GetNeighborTileAtMutable(Coords, Direction);
    if (NeighborTile == nullptr)
    {
        return;
    }

    auto& NeighborTileEdge = NeighborTile->Edges[Direction.Inverted().Index];
    NeighborTileEdge = NeighborTileEdge == TileEdgeType ? ETileEdgeType::Wall : TileEdgeType;
}

void STilemap::Excavate(UVec2Int Coords)
{
    if (!IsValidTile(Coords))
    {
        return;
    }
    auto Tile = GetTileAtMutable(Coords);
    Tile->Type = ETileType::Floor;

    for (SDirection::Type Direction = 0; Direction < SDirection::Count; ++Direction)
    {
        auto& TileEdge = Tile->Edges[Direction];

        auto NeighborTile = GetTileAtMutable(Coords + SDirection{ Direction }.GetVector<int>());
        if (NeighborTile != nullptr)
        {
            auto NeighborDirection = SDirection{ Direction }.Inverted();

            if (NeighborTile->Type == ETileType::Floor)
            {
                TileEdge = ETileEdgeType::Empty;

                NeighborTile->Edges[NeighborDirection.Index] = ETileEdgeType::Empty;
            }
            else
            {
                TileEdge = ETileEdgeType::Wall;

                NeighborTile->Edges[NeighborDirection.Index] = ETileEdgeType::Wall;
            }
        }
        else
        {
            TileEdge = ETileEdgeType::Wall;
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
    if (!IsValidTile(Coords))
    {
        return;
    }
    auto Tile = GetTileAtMutable(Coords);
    Tile->Type = ETileType::Empty;

    for (SDirection::Type Direction = 0; Direction < SDirection::Count; ++Direction)
    {
        auto& TileEdge = Tile->Edges[Direction];

        auto NeighborTile = GetTileAtMutable(Coords + SDirection{ Direction }.GetVector<int>());
        if (NeighborTile != nullptr)
        {
            auto NeighborDirection = SDirection{ Direction }.Inverted();

            if (NeighborTile->Type == ETileType::Floor)
            {
                TileEdge = ETileEdgeType::Wall;

                NeighborTile->Edges[NeighborDirection.Index] = ETileEdgeType::Wall;
            }
            else
            {
                TileEdge = ETileEdgeType::Empty;

                NeighborTile->Edges[NeighborDirection.Index] = ETileEdgeType::Empty;
            }
        }
        else
        {
            TileEdge = ETileEdgeType::Wall;
        }
    }
}

void STilemap::Serialize()
{

}

void SLevel::Update(float DeltaTime)
{
    DrawState.DoorInfo.Timeline.Advance(DeltaTime);
}

void SLevel::MarkDirty()
{
    DrawState.bDirty = true;
}
