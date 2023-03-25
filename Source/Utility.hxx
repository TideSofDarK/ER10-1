#pragma once

#include "glm/geometric.hpp"
#include "CommonTypes.hxx"

#define SIZE_OF_VECTOR_ELEMENT(Vector) sizeof(decltype(Vector)::value_type)

namespace Utility {
    float RotationFromDirection(EDirection Direction);

    float GetRandomFloat();

    float SmoothDamp(float Current, float Target, float SmoothTime, float MaxSpeed, float DeltaTime);

    template<typename T>
    constexpr auto InterpolateToConstant(T Current, T Target, float DeltaTime, float InterpSpeed) {
        if (InterpSpeed <= 0.0f) {
            return static_cast<T>(Target);
        }
        const auto Dist = glm::distance(Target, Current);
        if (Dist * Dist < 0.00001f) {
            return static_cast<T>(Target);
        }
        const auto Step = DeltaTime * InterpSpeed;
        return Current + glm::clamp((Target - Current), -Step, Step);
    }

    inline bool IsNumChar(char Char) {
        return Char >= '0' && Char <= '9';
    }

    float Pow10(int B);

    float FastAtoF(const char *&Start, const char *End);

    int FastAtoI(const char *&Start, const char *end);

    void ParseFloats(const char *Start, const char *End, float *Floats, int FloatCount);

    void ParseInts(const char *Start, const char *End, int *Ints, int IntCount);
}