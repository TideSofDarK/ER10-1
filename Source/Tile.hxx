#pragma once

#include <array>
#include "CommonTypes.hxx"

enum class ETileEdgeType
{
    Empty,
    Wall,
    Door
};

enum class ETileType
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
};
