#pragma once

#include "CommonTypes.hxx"
#include "Math.hxx"

enum class EPlayerAnimationType
{
    Idle,
    Turn,
    Walk,
    Enter,
    Bump
};

struct SBlob
{
    SDirection Direction{};
    UVec2Int Coords{};

    float InputBufferTime = 0.7f;

    EPlayerAnimationType AnimationType{};
    float AnimationAlpha{};

    float EyeHeight = 0.22f;

    UVec3 EyeForwardFrom{};
    UVec3 EyeForwardCurrent{};
    UVec3 EyeForwardTarget{};
    UVec3 EyePositionFrom{};
    UVec3 EyePositionCurrent{};
    UVec3 EyePositionTarget{};

    SBlob();

    void Update(float DeltaTime);

    void Turn(bool bRight);

    void ApplyDirection(bool bImmediate);

    void Step(UVec2Int DirectionVector);

    void BumpIntoWall();

    [[nodiscard]] bool IsMoving() const { return AnimationType != EPlayerAnimationType::Idle; }
    [[nodiscard]] bool IsReadyForBuffering() const { return AnimationAlpha > InputBufferTime && AnimationAlpha < 1.0f; }
};
