#pragma once

#include "glm/geometric.hpp"
#include "glm/mat4x4.hpp"
#include "CommonTypes.hxx"

#define SIZE_OF_VECTOR_ELEMENT(Vector) sizeof(decltype(Vector)::value_type)

namespace Utility {
    float RotationFromDirection(EDirection Direction);

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

inline bool IsNumChar(char Char) {
    return Char >= '0' && Char <= '9';
}

float Pow10(int n);

float FastAtoF(const char *&num, const char *end);

int FastAtoI(const char *&num, const char *end);

/** Expects numbers to be separated by 1 char */
void ParseFloats(const char *Start, const char *End, float *Floats, int FloatCount);

/** Expects numbers to be separated by 1 char */
void ParseInts(const char *Start, const char *End, int *Ints, int IntCount);
