#pragma once

#include <glm/glm.hpp>
#include "CommonTypes.hxx"

struct SPlayer {
    /** Allowed values are 0..3 */
    unsigned Direction: 2;
    int X{};
    int Y{};

    glm::vec3 EyeForwardCurrent{};
    glm::vec3 EyeForwardTarget{};
    glm::vec3 EyePositionCurrent{};
    glm::vec3 EyePositionTarget{};

    SPlayer();

    void HandleInput(SInputState InputState);

    void Update(float DeltaTime);

    void SetDirection(unsigned NewDirection, bool bImmediate);

    void SetDirection(EDirection NewDirection, bool bImmediate);
};
