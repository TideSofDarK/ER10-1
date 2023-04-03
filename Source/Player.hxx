#pragma once

#include <glm/glm.hpp>
#include "CommonTypes.hxx"
#include "Math.hxx"

struct SPlayer {
    SDirection Direction;
    UVec2Int Coords{};

    float EyeHeight = 0.22f;

    glm::vec3 EyeForwardCurrent{};
    glm::vec3 EyeForwardTarget{};
    glm::vec3 EyePositionCurrent{};
    glm::vec3 EyePositionTarget{};

    SPlayer();

    void HandleInput(SInputState InputState);

    void Update(float DeltaTime);

    void ApplyDirection(bool bImmediate);

    void MoveForward();
};
