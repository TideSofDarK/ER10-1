#include "Player.hxx"
#include "Utility.hxx"

#define EYE_HEIGHT 0.4f

void SPlayer::Update(float DeltaTime) {
    EyePositionCurrent = Utility::InterpolateToConstant(EyePositionCurrent, EyePositionTarget, DeltaTime, 4.0f);
    EyeForwardCurrent = Utility::InterpolateToConstant(EyeForwardCurrent, EyeForwardTarget, DeltaTime, 4.0f);
}

SPlayer::SPlayer() : Direction(0) {
    EyePositionCurrent = EyePositionTarget = {static_cast<float>(X), EYE_HEIGHT, static_cast<float>(Y)};
    SetDirection(Direction, true);
}

void SPlayer::HandleInput(const SInputState InputState) {
    if (InputState.Left == EKeyState::Pressed) {
        SetDirection(Direction - 1, false);
    } else if (InputState.Right == EKeyState::Pressed) {
        SetDirection(Direction + 1, false);
    }
    if (InputState.Up == EKeyState::Pressed) {
        EyePositionTarget += EyeForwardTarget;
    }
}

void SPlayer::SetDirection(unsigned NewDirection, bool bImmediate) {
    Direction = NewDirection;
    SetDirection(static_cast<EDirection>(Direction), bImmediate);
}

void SPlayer::SetDirection(EDirection NewDirection, bool bImmediate) {
    EyeForwardTarget = {0.0f, 0.0f, 0.0f};
    switch (NewDirection) {
        case EDirection::North:
            EyeForwardTarget.z += 1.0f;
            break;
        case EDirection::East:
            EyeForwardTarget.x -= 1.0f;
            break;
        case EDirection::South:
            EyeForwardTarget.z -= 1.0f;
            break;
        case EDirection::West:
            EyeForwardTarget.x += 1.0f;
            break;
        default:
            break;
    }
    if (bImmediate) {
        EyeForwardCurrent = EyeForwardTarget;
    }
}
