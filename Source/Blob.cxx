#include "Blob.hxx"

#include "Math.hxx"

void SBlob::PlayAnimation(EBlobAnimationType InAnimationType)
{
    switch (InAnimationType)
    {
        case EBlobAnimationType::Enter:
            Timeline.Speed = 1.25f;
            break;
        case EBlobAnimationType::Fall:
            Timeline.Speed = 0.5f;
            break;
        case EBlobAnimationType::Walk:
        default:
            Timeline.Speed = 3.3f;
            break;
    }
    AnimationType = InAnimationType;
    Timeline.Reset();
    bAnimationEndHandled = false;
}

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
        case EBlobAnimationType::Fall:
        {
            constexpr float InitialStep = 0.5f;
            constexpr float InitialJump = 0.5f;
            if (Timeline.Value < InitialStep)
            {
                EyePositionCurrent = UVec3::Mix(EyePositionFrom, UVec3::Mix(EyePositionFrom, EyePositionTarget, InitialStep), Math::EaseInBack(Timeline.Value / InitialStep));
            }
            else
            {
                EyePositionCurrent = UVec3::Mix(EyePositionFrom, EyePositionTarget, Timeline.Value);
                float Alpha = (Timeline.Value - InitialStep) / InitialJump;
                Alpha = Alpha * 3.0f - 1.0f;
                EyePositionCurrent.Y = EyePositionFrom.Y + (1 + (-Alpha * Alpha)) * 0.25f;
            }
        }
        break;
        case EBlobAnimationType::Idle:
        default:
            break;
    }
}

SBlob::SBlob()
{
    ResetEye();
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
        PlayAnimation(EBlobAnimationType::Turn);
        EyeForwardFrom = EyeForwardCurrent;
    }
}

void SBlob::Step(UVec2Int DirectionVector, EBlobAnimationType InAnimationType)
{
    if (AnimationType != EBlobAnimationType::Idle)
    {
        return;
    }
    PlayAnimation(InAnimationType);
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
    PlayAnimation(EBlobAnimationType::Bump);
    EyePositionFrom = EyePositionCurrent + (EyeForwardCurrent / 2.0f);
}

void SBlob::HijackRRF()
{
    MoveSeq.Moves[0] = SDirection::Right();
    MoveSeq.Moves[1] = SDirection::Right();
    MoveSeq.Moves[2] = SDirection::Forward();
    MoveSeq.ResetAndStart(3);
}

void SBlob::HijackRF()
{
    MoveSeq.Moves[0] = SDirection::Right();
    MoveSeq.Moves[1] = SDirection::Forward();
    MoveSeq.ResetAndStart(2);
}

void SBlob::HijackLF()
{
    MoveSeq.Moves[0] = SDirection::Left();
    MoveSeq.Moves[1] = SDirection::Forward();
    MoveSeq.ResetAndStart(2);
}

void SBlob::ResetEye()
{
    EyePositionCurrent = EyePositionTarget = { (float)Coords.X, EyeHeight, (float)Coords.Y };
}

EBlobAnimationType SBlob::HandleAnimationEnd()
{
    Timeline.Reset();
    auto OldAnimationType = AnimationType;
    AnimationType = EBlobAnimationType::Idle;
    bAnimationEndHandled = true;
    return OldAnimationType;
}
