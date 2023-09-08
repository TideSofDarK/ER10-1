#include "Level.hxx"

#include "Utility.hxx"

void SLevel::InitWallJoints() {
    WallJoints.fill(false);
    bUseWallJoints = true;
    UVec2Int Coords{};
    for (; Coords.X < Width; ++Coords.X) {
        for (Coords.Y = 0; Coords.Y < Height; ++Coords.Y) {
            auto CurrentTile = GetTileAtMutable(Coords);
            if (CurrentTile == nullptr) {
                continue;
            }
            /* @TODO: Treat doors as walls */
            if (CurrentTile->Edges[static_cast<int>(EDirection::North)] == ETileEdgeType::Wall &&
                CurrentTile->Edges[static_cast<int>(EDirection::West)] == ETileEdgeType::Wall) {
                if (auto WallJoint = GetWallJointAtMutable(Coords)) {
                    *WallJoint = true;
                }
            }
            if (CurrentTile->Edges[static_cast<int>(EDirection::North)] == ETileEdgeType::Wall &&
                CurrentTile->Edges[static_cast<int>(EDirection::East)] == ETileEdgeType::Wall) {
                if (auto WallJoint = GetWallJointAtMutable({Coords.X + 1, Coords.Y})) {
                    *WallJoint = true;
                }
            }
            if (CurrentTile->Edges[static_cast<int>(EDirection::South)] == ETileEdgeType::Wall &&
                CurrentTile->Edges[static_cast<int>(EDirection::East)] == ETileEdgeType::Wall) {
                if (auto WallJoint = GetWallJointAtMutable({Coords.X + 1, Coords.Y + 1})) {
                    *WallJoint = true;
                }
            }
            if (CurrentTile->Edges[static_cast<int>(EDirection::South)] == ETileEdgeType::Wall &&
                CurrentTile->Edges[static_cast<int>(EDirection::West)] == ETileEdgeType::Wall) {
                if (auto WallJoint = GetWallJointAtMutable({Coords.X, Coords.Y + 1})) {
                    *WallJoint = true;
                }
            }
        }
    }
}

void SLevel::Excavate(UVec2Int Coords) {
    if (!IsValidTile(Coords))
        return;
    auto Tile = GetTileAtMutable(Coords);
    Tile->Type = ETileType::Floor;

    for (unsigned Direction = 0; Direction < DIRECTION_COUNT; ++Direction) {
        auto &TileEdge = Tile->Edges[Direction];

        auto NeighborTile = GetTileAtMutable(Coords + SDirection(Direction).DirectionVectorFromDirection<int>());
        if (NeighborTile != nullptr) {
            auto NeighborDirection = SDirection(Direction);
            NeighborDirection.CycleCW();
            NeighborDirection.CycleCW();

            if (NeighborTile->Type == ETileType::Floor) {
                TileEdge = ETileEdgeType::Empty;

                NeighborTile->Edges[NeighborDirection.Index] = ETileEdgeType::Empty;
            } else {
                TileEdge = ETileEdgeType::Wall;

                NeighborTile->Edges[NeighborDirection.Index] = ETileEdgeType::Wall;
            }
        } else {
            TileEdge = ETileEdgeType::Wall;
        }
    }
}




