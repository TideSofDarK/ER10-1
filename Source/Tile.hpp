#pragma once

#include <array>
#include "CommonTypes.hpp"

namespace ETileEdge {
    enum Type {
        Empty,
        Wall,
        Door
    };
}

struct STile {
    std::array<ETileEdge::Type, EDirection::Count> Edges{};

    static STile Empty() {
        return STile{};
    };

    static STile WallNorth() { return STile{{ETileEdge::Wall}}; };

    static STile WallEast() { return STile{{ETileEdge::Empty, ETileEdge::Wall}}; };

    static STile WallSouth() { return STile{{ETileEdge::Empty, ETileEdge::Empty, ETileEdge::Wall}}; };

    static STile WallWest() {
        return STile{{ETileEdge::Empty, ETileEdge::Empty, ETileEdge::Empty, ETileEdge::Wall}};
    };

    static STile WallNorthWest() {
        return STile{{ETileEdge::Wall, ETileEdge::Empty, ETileEdge::Empty, ETileEdge::Wall}};
    };

    static STile WallNorthEast() {
        return STile{{ETileEdge::Wall, ETileEdge::Wall, ETileEdge::Empty, ETileEdge::Empty}};
    };

    static STile WallSouthWest() {
        return STile{{ETileEdge::Empty, ETileEdge::Empty, ETileEdge::Wall, ETileEdge::Wall}};
    };

    static STile WallSouthEast() {
        return STile{{ETileEdge::Empty, ETileEdge::Wall, ETileEdge::Wall, ETileEdge::Empty}};
    };
};
