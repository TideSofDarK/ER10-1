#include "Player.hxx"

constexpr float PI = 3.14159265358979323846f;

void SPlayer::Update(float DeltaTime) {
    AnimationAlpha += DeltaTime * 3.3f;
    if (AnimationAlpha >= 1.0f) {
        AnimationAlpha = 1.0f;
    }
    switch (AnimationType) {
        case EPlayerAnimationType::Turn:
            EyeForwardCurrent = UVec3::Mix(EyeForwardFrom, EyeForwardTarget, AnimationAlpha);
            break;
        case EPlayerAnimationType::Walk:
            EyePositionCurrent = UVec3::Mix(EyePositionFrom, EyePositionTarget, AnimationAlpha);
            break;
        case EPlayerAnimationType::Bump:
            EyePositionCurrent = UVec3::Mix(EyePositionTarget, EyePositionFrom,
                                            std::sin(AnimationAlpha * PI) * 0.25f);
            break;
        case EPlayerAnimationType::Idle:
        default:
            break;
    }
    if (AnimationAlpha >= 1.0f) {
        AnimationType = EPlayerAnimationType::Idle;
    }
}

SPlayer::SPlayer() : Direction(EDirection::North) {
    EyePositionCurrent = EyePositionTarget = {(float) Coords.X, EyeHeight, (float) Coords.Y};
    ApplyDirection(true);
}

void SPlayer::Turn(bool bRight) {
    if (AnimationType != EPlayerAnimationType::Idle)
        return;
    if (!bRight) {
        Direction.CycleCCW();
        ApplyDirection(false);
    } else {
        Direction.CycleCW();
        ApplyDirection(false);
    }
}

void SPlayer::ApplyDirection(bool bImmediate) {
    EyeForwardTarget = {0.0f, 0.0f, 0.0f};
    auto DirectionVector = Direction.DirectionVectorFromDirection<float>();
    EyeForwardTarget.X += DirectionVector.X;
    EyeForwardTarget.Z += DirectionVector.Y;

    if (bImmediate) {
        AnimationType = EPlayerAnimationType::Idle;
        AnimationAlpha = 1.0f;
        EyeForwardCurrent = EyeForwardTarget;
    } else {
        AnimationType = EPlayerAnimationType::Turn;
        AnimationAlpha = 0.0f;
        EyeForwardFrom = EyeForwardCurrent;
    }
}

void SPlayer::MoveForward() {
    if (AnimationType != EPlayerAnimationType::Idle)
        return;
    AnimationType = EPlayerAnimationType::Walk;
    AnimationAlpha = 0.0f;
    EyePositionFrom = EyePositionCurrent;
    EyePositionTarget += EyeForwardCurrent;
    Coords += Direction.DirectionVectorFromDirection<int>();
}

void SPlayer::BumpIntoWall() {
    if (AnimationType != EPlayerAnimationType::Idle)
        return;
    AnimationType = EPlayerAnimationType::Bump;
    AnimationAlpha = 0.0f;

    EyePositionFrom = EyePositionCurrent + (EyeForwardCurrent / 2.0f);
}
