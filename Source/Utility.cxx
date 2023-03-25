#include "Utility.hxx"
#include "glm/ext/matrix_transform.hpp"

#include <algorithm>
#include <random>

std::random_device RandomDevice;
std::mt19937 RNG(RandomDevice());
std::uniform_real_distribution<float> NormalizedDistribution(0.0f, 1.0f);

float Utility::RotationFromDirection(EDirection Direction) {
    switch (Direction) {
        case EDirection::East:
            return glm::pi<float>() * -0.5f;
        case EDirection::South:
            return glm::pi<float>();
        case EDirection::West:
            return glm::pi<float>() / 2.0f;
        default:
            return 0.0f;
    }
}

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

float Pow10(int n) {
    float ret = 1.0;
    float r = 10.0;
    if (n < 0) {
        n = -n;
        r = 0.1;
    }

    while (n) {
        if (n & 1) {
            ret *= r;
        }
        r *= r;
        n >>= 1;
    }
    return ret;
}

float FastAtoF(const char *&num, const char *const end) {
    const char *start = num;
    if (!num || !end || end <= num) {
        return 0;
    }

    int sign = 1;
    float int_part = 0.0;
    float frac_part = 0.0;
    bool has_frac = false;
    bool has_exp = false;

    if (*num == '-') {
        ++num;
        sign = -1;
    } else if (*num == '+') {
        ++num;
    }

    while (num != end) {
        if (IsNumChar(*num)) {
            int_part = int_part * 10 + (*num - '0');
        } else if (*num == '.') {
            has_frac = true;
            ++num;
            break;
        } else if (*num == 'e') {
            has_exp = true;
            ++num;
            break;
        } else {
            return sign * int_part;
        }
        ++num;
    }

    if (has_frac) {
        float frac_exp = 0.1;

        while (num != end) {
            if (IsNumChar(*num)) {
                frac_part += frac_exp * (*num - '0');
                frac_exp *= 0.1;
            } else if (*num == 'e') {
                has_exp = true;
                ++num;
                break;
            } else {
                return sign * (int_part + frac_part);
            }
            ++num;
        }
    }

    // parsing exponent part
    float exp_part = 1.0;
    if (num != end && has_exp) {
        int exp_sign = 1;
        if (*num == '-') {
            exp_sign = -1;
            ++num;
        } else if (*num == '+') {
            ++num;
        }

        int e = 0;
        while (num != end && *num >= '0' && *num <= '9') {
            e = e * 10 + *num - '0';
            ++num;
        }

        exp_part = Pow10(exp_sign * e);
    }

    return sign * (int_part + frac_part) * exp_part;
}

int FastAtoI(const char *&str, const char *end) {
    int Value = 0;
    while (str != end) {
        if (!IsNumChar(*str)) {
            return Value;
        }
        Value = Value * 10 + (*str++ - '0');
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
