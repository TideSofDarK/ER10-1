#pragma once

#include "CommonTypes.hxx"

struct SBlobMoveSeq
{
    int Current{};
    int Max{};
    SDirection Moves[4]{};

    SDirection GetCurrentDirection()
    {
        return Moves[Current];
    }

    void ResetAndStart(int NewMax)
    {
        Max = NewMax;
        Current = 0;
    }

    [[nodiscard]] inline bool IsFinished() const
    {
        return Current >= Max;
    }
};

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

    SBlobMoveSeq MoveSeq;

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

    void HijackRRF();
    void HijackRF();
    void HijackLF();

    [[nodiscard]] int GetExploreRadius() const { return 2; }
    [[nodiscard]] bool IsMoving() const { return AnimationType != EBlobAnimationType::Idle; }
    [[nodiscard]] bool IsReadyForBuffering() const { return Timeline.Value > InputBufferTime && Timeline.Value < 1.0f; }
};
