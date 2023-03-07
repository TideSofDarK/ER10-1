#include "Utility.hpp"

#include <algorithm>
#include <random>

static std::random_device Dev;
static std::mt19937 RNG(Dev());
static std::uniform_real_distribution<float> NormalizedDistribution(0.0f, 1.0f);

float Utility::GetRandomFloat() {
    return NormalizedDistribution(RNG);
}

float Utility::SmoothDamp(float Current, float Target, float SmoothTime, float MaxSpeed, float DeltaTime) {
    float CurrentVelocity = 0.0f;
    // Based on Game Programming Gems 4 Chapter 1.10
    SmoothTime = std::max(0.0001F, SmoothTime);
    float Omega = 2.0f / SmoothTime;

    float X = Omega * DeltaTime;
    float Exp = 1.0f / (1.0f + X + 0.48f * X * X + 0.235f * X * X * X);
    float Change = Current - Target;
    float OriginalTo = Target;

    // Clamp maximum speed
    float MaxChange = MaxSpeed * SmoothTime;
    Change = std::clamp(Change, -MaxChange, MaxChange);
    Target = Current - Change;

    float Temp = (CurrentVelocity + Omega * Change) * DeltaTime;
    CurrentVelocity = (CurrentVelocity - Omega * Temp) * Exp;
    float Output = Target + (Change + Temp) * Exp;

    // Prevent overshooting
    if (OriginalTo - Current > 0.0f == Output > OriginalTo) {
        Output = OriginalTo;
        CurrentVelocity = (Output - OriginalTo) / DeltaTime;
    }

    return Output;
}
