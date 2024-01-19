#include "Blob.hxx"

#include "Math.hxx"

constexpr float BlobAnimationSpeedDefault = 3.3f;
constexpr float BlobAnimationSpeedEnter = 1.25f;

void SBlob::Update(float DeltaTime)
{
    Timeline.Advance(DeltaTime);
    switch (AnimationType)
    {
        case EBlobAnimationType::Turn:
            EyeForwardCurrent = UVec3::Mix(EyeForwardFrom, EyeForwardTarget, Timeline.Value);
            break;
        case EBlobAnimationType::Walk:
            EyePositionCurrent = UVec3::Mix(EyePositionFrom, EyePositionTarget, Timeline.Value);
            break;
        case EBlobAnimationType::Bump:
            EyePositionCurrent = UVec3::Mix(EyePositionTarget, EyePositionFrom,
                std::sin(Timeline.Value * Math::PI) * 0.25f);
            break;
        case EBlobAnimationType::Enter:
            EyePositionCurrent = UVec3::Mix(EyePositionFrom, EyePositionTarget, Math::EaseInBack(Timeline.Value));
            break;
        case EBlobAnimationType::Idle:
        default:
            break;
    }
    if (Timeline.IsFinishedPlaying())
    {
        AnimationType = EBlobAnimationType::Idle;
    }
}

SBlob::SBlob()
{
    Timeline.Speed = BlobAnimationSpeedDefault;
    EyePositionCurrent = EyePositionTarget = { (float)Coords.X, EyeHeight, (float)Coords.Y };
    ApplyDirection(true);
}

void SBlob::Turn(bool bRight)
{
    if (AnimationType != EBlobAnimationType::Idle)
    {
        return;
    }
    if (!bRight)
    {
        Direction.CycleCCW();
        ApplyDirection(false);
    }
    else
    {
        Direction.CycleCW();
        ApplyDirection(false);
    }
}

void SBlob::ApplyDirection(bool bImmediate)
{
    EyeForwardTarget = { 0.0f, 0.0f, 0.0f };
    auto DirectionVector = Direction.GetVector<float>();
    EyeForwardTarget.X += DirectionVector.X;
    EyeForwardTarget.Z += DirectionVector.Y;

    if (bImmediate)
    {
        AnimationType = EBlobAnimationType::Idle;
        Timeline.Finish();
        EyeForwardCurrent = EyeForwardTarget;
    }
    else
    {
        AnimationType = EBlobAnimationType::Turn;
        Timeline.Speed = BlobAnimationSpeedDefault;
        Timeline.Reset();
        EyeForwardFrom = EyeForwardCurrent;
    }
}

void SBlob::Step(UVec2Int DirectionVector, bool bEnter)
{
    if (AnimationType != EBlobAnimationType::Idle)
    {
        return;
    }
    if (bEnter)
    {
        AnimationType = EBlobAnimationType::Enter;
        Timeline.Speed = BlobAnimationSpeedEnter;
    }
    else
    {
        AnimationType = EBlobAnimationType::Walk;
        Timeline.Speed = BlobAnimationSpeedDefault;
    }
    Timeline.Reset();
    EyePositionFrom = EyePositionCurrent;
    EyePositionTarget += UVec3{ (float)DirectionVector.X, 0.0f, (float)DirectionVector.Y };
    Coords += DirectionVector;
}

void SBlob::BumpIntoWall()
{
    if (AnimationType != EBlobAnimationType::Idle)
    {
        return;
    }
    AnimationType = EBlobAnimationType::Bump;
    Timeline.Reset();
    EyePositionFrom = EyePositionCurrent + (EyeForwardCurrent / 2.0f);
}
