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
    Bump,
    Fall
};

struct SBlob
{
private:
    STimeline Timeline{};
    EBlobAnimationType AnimationType{};
    bool bAnimationEndHandled = true;

    void PlayAnimation(EBlobAnimationType InAnimationType);

public:
    SDirection Direction{};
    UVec2Int Coords{};

    SBlobMoveSeq MoveSeq;

    float InputBufferTime = 0.7f;

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

    void Step(UVec2Int DirectionVector, EBlobAnimationType InAnimationType = EBlobAnimationType::Walk);

    void BumpIntoWall();

    void HijackRRF();
    void HijackRF();
    void HijackLF();

    void ResetEye();

    EBlobAnimationType HandleAnimationEnd();

    [[nodiscard]] SCoordsAndDirection UnreliableCoordsAndDirection() const
    {
        return {
            UnreliableCoords(),
            Direction
        };
    }
    [[nodiscard]] UVec2 UnreliableCoords() const { return UVec2{ EyePositionCurrent.X, EyePositionCurrent.Z }; }
    [[nodiscard]] bool IsMoving() const { return AnimationType != EBlobAnimationType::Idle; }
    [[nodiscard]] bool IsReadyForBuffering() const { return Timeline.Value > InputBufferTime && Timeline.Value < 1.0f; }
    [[nodiscard]] bool ShouldHandleAnimationEnd() const { return !bAnimationEndHandled && Timeline.IsFinishedPlaying(); }
};
