#include "Level.hxx"

void SLevel::InitWallJoints() {
    bUseWallJoints = true;
    UVec2Int Coords{};
    for (; Coords.X < Width; ++Coords.X) {
        for (Coords.Y = 0; Coords.Y < Height; ++Coords.Y) {
            auto CurrentTile = GetTileAtMutable(Coords);
            if (CurrentTile == nullptr) {
                continue;
            }
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




