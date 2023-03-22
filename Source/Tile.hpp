#pragma once

#include <array>
#include "CommonTypes.hpp"

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

    static STile Empty() {
        return STile{.Type = ETileType::Empty};
    };

    static STile WallsNoFloor() {
        return STile{{ETileEdgeType::Wall, ETileEdgeType::Wall, ETileEdgeType::Wall, ETileEdgeType::Wall},
                     ETileType::Empty};
    };

    static STile WallNorth() { return STile{{ETileEdgeType::Wall}}; };

    static STile WallEast() { return STile{{ETileEdgeType::Empty, ETileEdgeType::Wall}}; };

    static STile WallSouth() { return STile{{ETileEdgeType::Empty, ETileEdgeType::Empty, ETileEdgeType::Wall}}; };

    static STile WallWest() {
        return STile{{ETileEdgeType::Empty, ETileEdgeType::Empty, ETileEdgeType::Empty, ETileEdgeType::Wall}};
    };

    static STile WallNorthWest() {
        return STile{{ETileEdgeType::Wall, ETileEdgeType::Empty, ETileEdgeType::Empty, ETileEdgeType::Wall}};
    };

    static STile WallNorthEast() {
        return STile{{ETileEdgeType::Wall, ETileEdgeType::Wall, ETileEdgeType::Empty, ETileEdgeType::Empty}};
    };

    static STile WallSouthWest() {
        return STile{{ETileEdgeType::Empty, ETileEdgeType::Empty, ETileEdgeType::Wall, ETileEdgeType::Wall}};
    };

    static STile WallSouthEast() {
        return STile{{ETileEdgeType::Empty, ETileEdgeType::Wall, ETileEdgeType::Wall, ETileEdgeType::Empty}};
    };
};
