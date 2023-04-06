#pragma once

#include "CommonTypes.hxx"
#include "Math.hxx"

enum class EPlayerAnimationType {
    Idle,
    Turn,
    Walk,
    Bump
};

struct SPlayer {
    SDirection Direction;
    UVec2Int Coords{};

    EPlayerAnimationType AnimationType{};
    float AnimationAlpha{};

    float EyeHeight = 0.22f;

    UVec3 EyeForwardFrom{};
    UVec3 EyeForwardCurrent{};
    UVec3 EyeForwardTarget{};
    UVec3 EyePositionFrom{};
    UVec3 EyePositionCurrent{};
    UVec3 EyePositionTarget{};

    SPlayer();

    void Update(float DeltaTime);

    void Turn(bool bRight);

    void ApplyDirection(bool bImmediate);

    void MoveForward();

    void BumpIntoWall();

    [[nodiscard]] bool IsMoving() const { return AnimationType != EPlayerAnimationType::Idle; }
};
