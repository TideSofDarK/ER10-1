#pragma once

#include <array>
#include "CommonTypes.hxx"

enum class ETileEdgeType {
    Empty,
    Wall,
    Door
};

enum class ETileType {
    Empty,
    Floor,
    Hole
};

struct STile {
    std::array<ETileEdgeType, DIRECTION_COUNT> Edges{};

    ETileType Type{ETileType::Floor};

    [[nodiscard]] bool IsWalkable() const {
        return Type != ETileType::Empty;
    }

    [[nodiscard]] bool IsTraversable(unsigned Direction) const {
        return Edges[Direction] != ETileEdgeType::Wall;
    }

    static STile Empty() {
        return STile{.Type = ETileType::Empty};
    };

    static STile WallsNF() {
        return STile{{ETileEdgeType::Wall, ETileEdgeType::Wall, ETileEdgeType::Wall, ETileEdgeType::Wall},
                     ETileType::Empty};
    };

    static STile WallN() { return STile{{ETileEdgeType::Wall}}; };

    static STile WallE() { return STile{{ETileEdgeType::Empty, ETileEdgeType::Wall}}; };

    static STile WallS() { return STile{{ETileEdgeType::Empty, ETileEdgeType::Empty, ETileEdgeType::Wall}}; };

    static STile WallW() {
        return STile{{ETileEdgeType::Empty, ETileEdgeType::Empty, ETileEdgeType::Empty, ETileEdgeType::Wall}};
    };

    static STile WallWE() {
        return STile{{ETileEdgeType::Empty, ETileEdgeType::Wall, ETileEdgeType::Empty, ETileEdgeType::Wall}};
    };

    static STile WallNW() {
        return STile{{ETileEdgeType::Wall, ETileEdgeType::Empty, ETileEdgeType::Empty, ETileEdgeType::Wall}};
    };

    static STile WallNE() {
        return STile{{ETileEdgeType::Wall, ETileEdgeType::Wall, ETileEdgeType::Empty, ETileEdgeType::Empty}};
    };

    static STile WallSW() {
        return STile{{ETileEdgeType::Empty, ETileEdgeType::Empty, ETileEdgeType::Wall, ETileEdgeType::Wall}};
    };

    static STile WallSE() {
        return STile{{ETileEdgeType::Empty, ETileEdgeType::Wall, ETileEdgeType::Wall, ETileEdgeType::Empty}};
    };

    static STile WallNWS() {
        return STile{{ETileEdgeType::Wall, ETileEdgeType::Empty, ETileEdgeType::Wall, ETileEdgeType::Wall}};
    };

    static STile WallNEW() {
        return STile{{ETileEdgeType::Wall, ETileEdgeType::Wall, ETileEdgeType::Empty, ETileEdgeType::Wall}};
    };
};
