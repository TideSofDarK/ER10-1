#pragma once

#include "Math.hxx"

struct SWindowData {
    int Width{};
    int Height{};
    int BlitWidth{};
    int BlitHeight{};
    int BlitX{};
    int BlitY{};
    unsigned long long Last{};
    unsigned long long Now{};
    float Seconds{};
    float DeltaTime{};
    bool bQuit{};
};

enum class EDirection : unsigned {
    North = 0,
    East,
    South,
    West,
    Count
};

struct SDirection {
    unsigned Direction: 2;

    explicit SDirection(unsigned InDirection) : Direction(InDirection) {}

    SDirection(EDirection InDirection) : Direction(static_cast<unsigned>(InDirection)) {}

    EDirection GetEnum() const { return static_cast<EDirection>(Direction); }

//    bool operator==(const SDirection &Other) const {
//        return Direction == Other.Direction;
//    }
//
//    bool operator==(const int &InDirection) const {
//        return Direction == InDirection;
//    }
//
//    SDirection &operator=(int InDirection) {
//        Direction = InDirection;
//        return *this;
//    }

    void CycleCW() {
        Direction++;
    }

    void CycleCCW() {
        Direction--;
    }

    template<typename T>
    SVec2<T> DirectionVectorFromDirection() const {
        switch (GetEnum()) {
            case EDirection::North:
                return {0, 1};
            case EDirection::East:
                return {-1, 0};
            case EDirection::South:
                return {0, -1};
            case EDirection::West:
                return {1, 0};
            default:
                return {};
        }
    }

    [[nodiscard]] float RotationFromDirection() const {
        switch (GetEnum()) {
            case EDirection::East:
                return Math::PI * -0.5f;
            case EDirection::South:
                return Math::PI;
            case EDirection::West:
                return Math::PI / 2.0f;
            default:
                return 0.0f;
        }
    }
};

#define DIRECTION_COUNT static_cast<unsigned>(EDirection::Count)

enum class EKeyState : unsigned {
    None,
    Pressed,
    Press,
    Released
};

struct SInputState {
    EKeyState Up: 2;
    EKeyState Right: 2;
    EKeyState Down: 2;
    EKeyState Left: 2;
    EKeyState Accept: 2;
    EKeyState Cancel: 2;
    EKeyState L: 2;
    EKeyState ZL: 2;
    EKeyState R: 2;
    EKeyState ZR: 2;

    EKeyState ToggleFullscreen: 2;
};
