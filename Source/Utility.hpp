#pragma once

#include "glm/geometric.hpp"

namespace Utility {
    float GetRandomFloat();

    float SmoothDamp(float Current, float Target, float SmoothTime, float MaxSpeed, float DeltaTime);

    template<typename genType>
    constexpr auto InterpolateToConstant(genType Current, genType Target, float DeltaTime, float InterpSpeed) {
        if (InterpSpeed <= 0.0f) {
            return static_cast<genType>(Target);
        }
        const auto Dist = glm::distance(Target, Current);
        if (Dist * Dist < 0.00001f) {
            return static_cast<genType>(Target);
        }
        const auto Step = DeltaTime * InterpSpeed;
        return Current + glm::clamp((Target - Current), -Step, Step);
    }
}