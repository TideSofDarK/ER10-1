#include <cstdio>
#include "Player.hxx"
#include "Utility.hxx"

void SPlayer::Update(float DeltaTime) {
    EyePositionCurrent = Utility::InterpolateToConstant(EyePositionCurrent, EyePositionTarget, DeltaTime, 4.0f);
    EyeForwardCurrent = Utility::InterpolateToConstant(EyeForwardCurrent, EyeForwardTarget, DeltaTime, 4.0f);
}

SPlayer::SPlayer() : Direction(EDirection::North) {
    EyePositionCurrent = EyePositionTarget = {static_cast<float>(Coords.X), EyeHeight, static_cast<float>(Coords.Y)};
    ApplyDirection(true);
}

void SPlayer::HandleInput(const SInputState InputState) {
    if (InputState.Left == EKeyState::Pressed) {
        Direction.CycleCCW();
        ApplyDirection(false);
    } else if (InputState.Right == EKeyState::Pressed) {
        Direction.CycleCW();
        ApplyDirection(false);
    }
    if (InputState.Up == EKeyState::Pressed) {
        EyePositionTarget += EyeForwardTarget;
        Coords += Direction.DirectionVectorFromDirection<int>();
    }
}

void SPlayer::ApplyDirection(bool bImmediate) {
    EyeForwardTarget = {0.0f, 0.0f, 0.0f};
    auto DirectionVector = Direction.DirectionVectorFromDirection<float>();
    EyeForwardTarget.x += DirectionVector.X;
    EyeForwardTarget.z += DirectionVector.Y;
    if (bImmediate) {
        EyeForwardCurrent = EyeForwardTarget;
    }
}
