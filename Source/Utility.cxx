#include "Utility.hxx"

#include <algorithm>
#include <random>

namespace Utility {
    std::random_device RandomDevice;
    std::mt19937 RNG(RandomDevice());
    std::uniform_real_distribution<float> NormalizedDistribution(0.0f, 1.0f);

    float GetRandomFloat() {
        return NormalizedDistribution(RNG);
    }

    float SmoothDamp(float Current, float Target, float SmoothTime, float MaxSpeed, float DeltaTime) {
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

    float Pow10(int B) {
        float Value = 1.0;
        float A = 10.0;
        if (B < 0) {
            B = -B;
            A = 0.1;
        }

        while (B) {
            if (B & 1) {
                Value *= A;
            }
            A *= A;
            B >>= 1;
        }
        return Value;
    }

    float FastAtoF(const char *&Start, const char *const End) {
        if (!Start || !End || End <= Start) {
            return 0;
        }

        float Sign = 1.0f;
        float IntPart = 0.0f;
        float FracPart = 0.0f;
        bool bHasFrac = false;
        bool bHasExp = false;

        if (*Start == '-') {
            ++Start;
            Sign = -1.0f;
        } else if (*Start == '+') {
            ++Start;
        }

        while (Start != End) {
            if (IsNumChar(*Start)) {
                IntPart = IntPart * 10.0f + static_cast<float>(*Start - '0');
            } else if (*Start == '.') {
                bHasFrac = true;
                ++Start;
                break;
            } else if (*Start == 'e') {
                bHasExp = true;
                ++Start;
                break;
            } else {
                return Sign * IntPart;
            }
            ++Start;
        }

        if (bHasFrac) {
            float FracExp = 0.1f;

            while (Start != End) {
                if (IsNumChar(*Start)) {
                    FracPart += FracExp * static_cast<float>(*Start - '0');
                    FracExp *= 0.1f;
                } else if (*Start == 'e') {
                    bHasExp = true;
                    ++Start;
                    break;
                } else {
                    return Sign * (IntPart + FracPart);
                }
                ++Start;
            }
        }

        float ExpPart = 1.0f;
        if (Start != End && bHasExp) {
            int ExpSign = 1;
            if (*Start == '-') {
                ExpSign = -1;
                ++Start;
            } else if (*Start == '+') {
                ++Start;
            }

            int e = 0;
            while (Start != End && *Start >= '0' && *Start <= '9') {
                e = e * 10 + *Start - '0';
                ++Start;
            }

            ExpPart = Pow10(ExpSign * e);
        }

        return Sign * (IntPart + FracPart) * ExpPart;
    }

    int FastAtoI(const char *&Start, const char *end) {
        int Value = 0;
        while (Start != end) {
            if (!IsNumChar(*Start)) {
                return Value;
            }
            Value = Value * 10 + (*Start++ - '0');
        }
        return Value;
    }

    void ParseFloats(const char *Start, const char *End, float *Floats, int FloatCount) {
        for (int Index = 0; Index < FloatCount; ++Index) {
            Floats[Index] = FastAtoF(Start, End);
            Start++;
        }
    }

    void ParseInts(const char *Start, const char *End, int *Ints, int IntCount) {
        for (int Index = 0; Index < IntCount; ++Index) {
            Ints[Index] = FastAtoI(Start, End);
            Start++;
        }
    }
}
