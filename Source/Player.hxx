#pragma once

#include <glm/glm.hpp>
#include "CommonTypes.hxx"
#include "Math.hxx"

enum class EPlayerAnimationType {
    Idle,
    Turn,
    Walk,
    Bump
};

struct SPlayer {
    SDirection Direction;
    UVec2Int Coords{};

    EPlayerAnimationType AnimationType{};
    float AnimationAlpha{};

    float EyeHeight = 0.22f;

    glm::vec3 EyeForwardFrom{};
    glm::vec3 EyeForwardCurrent{};
    glm::vec3 EyeForwardTarget{};
    glm::vec3 EyePositionFrom{};
    glm::vec3 EyePositionCurrent{};
    glm::vec3 EyePositionTarget{};

    SPlayer();

    void Update(float DeltaTime);

    void Turn(bool bRight);

    void ApplyDirection(bool bImmediate);

    void MoveForward();

    void BumpIntoWall();

    [[nodiscard]] bool IsMoving() const { return AnimationType != EPlayerAnimationType::Idle; }
};
