#pragma once

#include "Serialization.hxx"
#include "CommonTypes.hxx"
#include "SharedConstants.hxx"

using ETileFlag = UFlagType;
using ETileSpecialFlag = UFlagType;
using ETileEdgeFlag = UFlagType;
using ETileSpecialEdgeFlag = UFlagType;

struct STile
{
    ETileFlag Flags{};
    ETileSpecialFlag SpecialFlags{};
    ETileEdgeFlag EdgeFlags{};
    ETileSpecialEdgeFlag SpecialEdgeFlags{};

    [[nodiscard]] static constexpr UFlagType DirectionBit(UFlagType NorthBit, SDirection Direction)
    {
        return NorthBit << Direction.Index;
    }

    [[nodiscard]] inline bool CheckFlag(ETileFlag Flag) const
    {
        return Flags & Flag;
    }

    [[nodiscard]] inline bool CheckEdgeFlag(ETileEdgeFlag NorthBit, SDirection Direction) const
    {
        return EdgeFlags & DirectionBit(NorthBit, Direction);
    }

    [[nodiscard]] inline bool IsEdgeEmpty(SDirection Direction) const
    {
        return !(EdgeFlags & DirectionBit(TILE_EDGE_NORTH_BITS, Direction));
    }

    inline void SetSpecialFlag(ETileSpecialFlag Flag)
    {
        SpecialFlags |= Flag;
    }

    inline void ClearSpecialFlag(ETileSpecialFlag Flag)
    {
        SpecialFlags &= ~Flag;
    }

    inline void SetEdgeFlag(ETileEdgeFlag EdgeBit, SDirection Direction)
    {
        ClearEdgeFlags(Direction);
        EdgeFlags |= DirectionBit(EdgeBit, Direction);
    }

    inline void ClearEdgeFlags(SDirection Direction)
    {
        EdgeFlags &= ~DirectionBit(TILE_EDGE_NORTH_BITS, Direction);
    }

    inline void SetWall(SDirection Direction)
    {
        EdgeFlags |= DirectionBit(TILE_EDGE_WALL_BIT, Direction);
    }

    [[nodiscard]] inline bool IsWalkable() const
    {
        return CheckFlag(TILE_FLOOR_BIT) || CheckFlag(TILE_HOLE_BIT);
    }

    [[nodiscard]] inline bool IsEdgeTraversable(SDirection Direction) const
    {
        return !(EdgeFlags & DirectionBit(TILE_EDGE_WALL_BIT, Direction));
    }

    [[nodiscard]] inline bool IsDoorEdge(SDirection Direction) const
    {
        return CheckEdgeFlag(TILE_EDGE_DOOR_BIT, Direction);
    }

    [[nodiscard]] bool IsWallBasedEdge(SDirection Direction) const
    {
        return CheckEdgeFlag(TILE_EDGE_WALL_BIT, Direction) || CheckEdgeFlag(TILE_EDGE_DOOR_BIT, Direction);
    }

    static STile Floor()
    {
        STile Tile;
        Tile.Flags = TILE_FLOOR_BIT;
        return Tile;
    }

    static STile WallsNF()
    {
        STile Tile;
        Tile.EdgeFlags = TILE_EDGE_WALL_BIT | TILE_EDGE_WALL_EAST_BIT | TILE_EDGE_WALL_SOUTH_BIT | TILE_EDGE_WALL_WEST_BIT;
        return Tile;
    }

    static STile WallN()
    {
        STile Tile;
        Tile.Flags = TILE_FLOOR_BIT;
        Tile.EdgeFlags = TILE_EDGE_WALL_BIT;
        return Tile;
    };

    static STile WallE()
    {
        STile Tile;
        Tile.Flags = TILE_FLOOR_BIT;
        Tile.EdgeFlags = TILE_EDGE_WALL_EAST_BIT;
        return Tile;
    };

    static STile WallS()
    {
        STile Tile;
        Tile.Flags = TILE_FLOOR_BIT;
        Tile.EdgeFlags = TILE_EDGE_WALL_SOUTH_BIT;
        return Tile;
    };

    static STile WallW()
    {
        STile Tile;
        Tile.Flags = TILE_FLOOR_BIT;
        Tile.EdgeFlags = TILE_EDGE_WALL_WEST_BIT;
        return Tile;
    }

    static STile WallWE()
    {
        STile Tile;
        Tile.Flags = TILE_FLOOR_BIT;
        Tile.EdgeFlags = TILE_EDGE_WALL_WEST_BIT | TILE_EDGE_WALL_EAST_BIT;
        return Tile;
    }

    static STile WallNS()
    {
        STile Tile;
        Tile.Flags = TILE_FLOOR_BIT;
        Tile.EdgeFlags = TILE_EDGE_WALL_BIT | TILE_EDGE_WALL_SOUTH_BIT;
        return Tile;
    }

    static STile WallNW()
    {
        STile Tile;
        Tile.Flags = TILE_FLOOR_BIT;
        Tile.EdgeFlags = TILE_EDGE_WALL_BIT | TILE_EDGE_WALL_WEST_BIT;
        return Tile;
    }

    static STile WallNE()
    {
        STile Tile;
        Tile.Flags = TILE_FLOOR_BIT;
        Tile.EdgeFlags = TILE_EDGE_WALL_BIT | TILE_EDGE_WALL_EAST_BIT;
        return Tile;
    }

    static STile WallSW(bool bHasFloor = true)
    {
        STile Tile;
        Tile.Flags = bHasFloor ? TILE_FLOOR_BIT : 0;
        Tile.EdgeFlags = TILE_EDGE_WALL_SOUTH_BIT | TILE_EDGE_WALL_WEST_BIT;
        return Tile;
    }

    static STile WallSE()
    {
        STile Tile;
        Tile.Flags = TILE_FLOOR_BIT;
        Tile.EdgeFlags = TILE_EDGE_WALL_SOUTH_BIT | TILE_EDGE_WALL_EAST_BIT;
        return Tile;
    }

    static STile WallNWS()
    {
        STile Tile;
        Tile.Flags = TILE_FLOOR_BIT;
        Tile.EdgeFlags = TILE_EDGE_WALL_SOUTH_BIT | TILE_EDGE_WALL_BIT | TILE_EDGE_WALL_WEST_BIT;
        return Tile;
    }

    static STile WallSWE()
    {
        STile Tile;
        Tile.Flags = TILE_FLOOR_BIT;
        Tile.EdgeFlags = TILE_EDGE_WALL_SOUTH_BIT | TILE_EDGE_WALL_WEST_BIT | TILE_EDGE_WALL_EAST_BIT;
        return Tile;
    }

    static STile WallNEW()
    {
        STile Tile;
        Tile.Flags = TILE_FLOOR_BIT;
        Tile.EdgeFlags = TILE_EDGE_WALL_WEST_BIT | TILE_EDGE_WALL_BIT | TILE_EDGE_WALL_EAST_BIT;
        return Tile;
    }

    static STile WallWDoorS()
    {
        STile Tile;
        Tile.Flags = TILE_FLOOR_BIT;
        Tile.EdgeFlags = TILE_EDGE_WALL_WEST_BIT | TILE_EDGE_DOOR_SOUTH_BIT;
        return Tile;
    }

    static STile WallEWDoorN()
    {
        STile Tile;
        Tile.Flags = TILE_FLOOR_BIT;
        Tile.EdgeFlags = TILE_EDGE_WALL_WEST_BIT | TILE_EDGE_WALL_EAST_BIT | TILE_EDGE_DOOR_BIT;
        return Tile;
    }

    void Serialize(std::ofstream& Stream) const
    {
        Serialization::Write32(Stream, Flags);
        Serialization::Write32(Stream, SpecialFlags);
        Serialization::Write32(Stream, EdgeFlags);
        Serialization::Write32(Stream, SpecialEdgeFlags);
    }

    void Deserialize(std::istream& Stream)
    {
        Serialization::Read32(Stream, Flags);
        Serialization::Read32(Stream, SpecialFlags);
        Serialization::Read32(Stream, EdgeFlags);
        Serialization::Read32(Stream, SpecialEdgeFlags);
    }
};
