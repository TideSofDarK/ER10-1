#pragma once

#include <array>
#include <optional>
#include "Serialization.hxx"
#include "CommonTypes.hxx"

enum class ETileEdgeType : uint32_t
{
    Empty,
    Wall,
    Door
};

enum class ETileType : uint32_t
{
    Empty,
    Floor,
    Hole
};

struct STile
{
    std::array<ETileEdgeType, SDirection::Count> Edges{};

    ETileType Type{ ETileType::Empty };

    [[nodiscard]] bool IsWalkable() const
    {
        return Type != ETileType::Empty;
    }

    [[nodiscard]] bool IsTraversable(unsigned Direction) const
    {
        return Edges[Direction] != ETileEdgeType::Wall;
    }

    [[nodiscard]] bool IsDoorEdge(SDirection Direction) const
    {
        return Edges[Direction.Index] == ETileEdgeType::Door;
    }

    [[nodiscard]] bool IsWallBasedEdge(SDirection Direction) const
    {
        return Edges[Direction.Index] == ETileEdgeType::Wall || Edges[Direction.Index] == ETileEdgeType::Door;
    }

    static STile Floor()
    {
        STile Tile;
        Tile.Type = ETileType::Floor;
        return Tile;
    }

    static STile WallsNF()
    {
        return STile{ { ETileEdgeType::Wall, ETileEdgeType::Wall, ETileEdgeType::Wall, ETileEdgeType::Wall },
            ETileType::Empty };
    }

    static STile WallN() { return STile{ { ETileEdgeType::Wall }, ETileType::Floor }; };

    static STile WallE() { return STile{ { ETileEdgeType::Empty, ETileEdgeType::Wall }, ETileType::Floor }; };

    static STile WallS()
    {
        return STile{ { ETileEdgeType::Empty, ETileEdgeType::Empty, ETileEdgeType::Wall }, ETileType::Floor };
    };

    static STile WallW()
    {
        return STile{ { ETileEdgeType::Empty, ETileEdgeType::Empty, ETileEdgeType::Empty, ETileEdgeType::Wall },
            ETileType::Floor };
    }

    static STile WallWE()
    {
        return STile{ { ETileEdgeType::Empty, ETileEdgeType::Wall, ETileEdgeType::Empty, ETileEdgeType::Wall },
            ETileType::Floor };
    }

    static STile WallNS()
    {
        return STile{ { ETileEdgeType::Wall, ETileEdgeType::Empty, ETileEdgeType::Wall, ETileEdgeType::Empty },
            ETileType::Floor };
    }

    static STile WallNW()
    {
        return STile{ { ETileEdgeType::Wall, ETileEdgeType::Empty, ETileEdgeType::Empty, ETileEdgeType::Wall },
            ETileType::Floor };
    }

    static STile WallNE()
    {
        return STile{ { ETileEdgeType::Wall, ETileEdgeType::Wall, ETileEdgeType::Empty, ETileEdgeType::Empty },
            ETileType::Floor };
    }

    static STile WallSW(bool bHasFloor = true)
    {
        return STile{ { ETileEdgeType::Empty, ETileEdgeType::Empty, ETileEdgeType::Wall, ETileEdgeType::Wall },
            bHasFloor ? ETileType::Floor : ETileType::Empty };
    }

    static STile WallSE()
    {
        return STile{ { ETileEdgeType::Empty, ETileEdgeType::Wall, ETileEdgeType::Wall, ETileEdgeType::Empty },
            ETileType::Floor };
    }

    static STile WallNWS()
    {
        return STile{ { ETileEdgeType::Wall, ETileEdgeType::Empty, ETileEdgeType::Wall, ETileEdgeType::Wall },
            ETileType::Floor };
    }

    static STile WallSWE()
    {
        return STile{ { ETileEdgeType::Empty, ETileEdgeType::Wall, ETileEdgeType::Wall, ETileEdgeType::Wall },
            ETileType::Floor };
    }

    static STile WallNEW()
    {
        return STile{ { ETileEdgeType::Wall, ETileEdgeType::Wall, ETileEdgeType::Empty, ETileEdgeType::Wall },
            ETileType::Floor };
    }

    void Serialize(std::ofstream& Stream) const
    {
        Serialization::Write32(Stream, (uint32_t)Edges[0]);
        Serialization::Write32(Stream, (uint32_t)Edges[1]);
        Serialization::Write32(Stream, (uint32_t)Edges[2]);
        Serialization::Write32(Stream, (uint32_t)Edges[3]);

        Serialization::Write32(Stream, (uint32_t)Type);
    }

    void Deserialize(std::ifstream& Stream)
    {
        Serialization::Read32(Stream, Edges[0]);
        Serialization::Read32(Stream, Edges[1]);
        Serialization::Read32(Stream, Edges[2]);
        Serialization::Read32(Stream, Edges[3]);

        Serialization::Read32(Stream, Type);
    }
};
