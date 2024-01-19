#pragma once

#include "CommonTypes.hxx"

enum class EBlobAnimationType
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

    EBlobAnimationType AnimationType{};
    STimeline Timeline{};

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

    void Step(UVec2Int DirectionVector, bool bEnter = false);

    void BumpIntoWall();

    [[nodiscard]] bool IsMoving() const { return AnimationType != EBlobAnimationType::Idle; }
    [[nodiscard]] bool IsReadyForBuffering() const { return Timeline.Value > InputBufferTime && Timeline.Value < 1.0f; }
};
