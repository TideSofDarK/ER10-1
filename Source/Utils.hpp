#pragma once

#include <algorithm>

namespace Utils {
    constexpr float SmoothDamp(float current, float target, float smoothTime, float maxSpeed, float deltaTime) {
        float currentVelocity = 0.0f;
        // Based on Game Programming Gems 4 Chapter 1.10
        smoothTime = std::max(0.0001F, smoothTime);
        float omega = 2.0f / smoothTime;

        float x = omega * deltaTime;
        float exp = 1.0f / (1.0f + x + 0.48f * x * x + 0.235f * x * x * x);
        float change = current - target;
        float originalTo = target;

        // Clamp maximum speed
        float maxChange = maxSpeed * smoothTime;
        change = std::clamp(change, -maxChange, maxChange);
        target = current - change;

        float temp = (currentVelocity + omega * change) * deltaTime;
        currentVelocity = (currentVelocity - omega * temp) * exp;
        float output = target + (change + temp) * exp;

        // Prevent overshooting
        if (originalTo - current > 0.0f == output > originalTo) {
            output = originalTo;
            currentVelocity = (output - originalTo) / deltaTime;
        }

        return output;
    }

    template<typename genType>
    constexpr static auto InterpolateToConstant(genType Current, genType Target, float DeltaTime, float InterpSpeed) {
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