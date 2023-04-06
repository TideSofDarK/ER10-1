#include "Player.hxx"

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
        case EPlayerAnimationType::Bump:
            EyePositionCurrent = UVec3::Mix(EyePositionFrom, EyePositionTarget, AnimationAlpha);
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
    EyePositionCurrent = EyePositionTarget = {static_cast<float>(Coords.X), EyeHeight, static_cast<float>(Coords.Y)};
    ApplyDirection(true);
}

void SPlayer::Turn(bool bRight) {
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
    AnimationType = EPlayerAnimationType::Walk;
    AnimationAlpha = 0.0f;
    EyePositionFrom = EyePositionCurrent;
    EyePositionTarget += EyeForwardCurrent;
    Coords += Direction.DirectionVectorFromDirection<int>();
}

void SPlayer::BumpIntoWall() {
    AnimationType = EPlayerAnimationType::Bump;
    AnimationAlpha = 0.0f;
}
