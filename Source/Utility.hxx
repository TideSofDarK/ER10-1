#pragma once

#include <cstddef>
#include <cstdint>

#define SIZE_OF_VECTOR_ELEMENT(Vector) sizeof(decltype(Vector)::value_type)

namespace Utility
{
    float GetRandomFloat();

    float SmoothDamp(float Current, float Target, float SmoothTime, float MaxSpeed, float DeltaTime);

    //    template<typename T>
    //    constexpr auto InterpolateToConstant(T Current, T Target, float DeltaTime, float InterpSpeed) {
    //        if (InterpSpeed <= 0.0f) {
    //            return static_cast<T>(Target);
    //        }
    //        const auto Dist = glm::distance(Target, Current);
    //        if (Dist * Dist < 0.00001f) {
    //            return static_cast<T>(Target);
    //        }
    //        const auto Step = DeltaTime * InterpSpeed;
    //        return Current + glm::clamp((Target - Current), -Step, Step);
    //    }

    inline bool IsNumChar(char Char)
    {
        return Char >= '0' && Char <= '9';
    }

    float Pow10(int B);

    float FastAtoF(const char*& Start, const char* End);

    int FastAtoI(const char*& Start, const char* end);

    void ParseFloats(const char* Start, const char* End, float* Floats, int FloatCount);

    void ParseInts(const char* Start, const char* End, int* Ints, int IntCount);

    static constexpr std::size_t DoAlign(std::size_t Num, std::size_t Alignment)
    {
        return (Num + (Alignment - 1)) & ~(Alignment - 1);
    }

    static inline void* AlignPtr(void* Ptr, std::size_t Alignment)
    {
        return (void*)(DoAlign((size_t)Ptr, Alignment));
    }

    static inline bool IsAlignedPtr(void* Ptr, std::size_t Alignment)
    {
        return ((std::size_t)Ptr & (Alignment - 1)) == 0;
    }

    static inline bool IsBigEndian()
    {
        union
        {
            uint32_t I;
            char C[4];
        } Bint = { 0x01020304 };

        return Bint.C[0] == 1;
    }

    auto HtoNS = [](uint16_t H) {
        return (uint16_t)(H << 8 & 0xFF00U | H >> 8 & 0x00FFU);
    };

    auto HtoNL = [](uint32_t H) {
        return (uint32_t)(H << 24 & 0xFF000000U | H << 8 & 0x00FF0000U | H >> 8 & 0x0000FF00U | H >> 24 & 0x000000FFU);
    };

    auto NtoHS = HtoNS;
    auto NtoHL = HtoNL;
}
