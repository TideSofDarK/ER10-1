#pragma once

#include <cmath>
#include <algorithm>
#include <type_traits>

template <typename T>
struct SVec2
{
    using Type = T;

    T X{};
    T Y{};

    constexpr SVec2() = default;

    constexpr SVec2(T InX, T InY)
        : X(InX), Y(InY){};

    explicit constexpr SVec2(T InValue)
        : X(InValue), Y(InValue){};

    template <typename AnotherT, typename = typename std::enable_if<std::is_same<T, AnotherT>::value == false>::type>
    explicit constexpr SVec2(const SVec2<AnotherT>& Another)
    {
        X = static_cast<T>(Another.X);
        Y = static_cast<T>(Another.Y);
    }

    constexpr SVec2<T> operator-(const SVec2<T>& Other) const
    {
        return { X - Other.X, Y - Other.Y };
    }

    constexpr SVec2<T> operator+(const SVec2<T>& Other) const
    {
        return { X + Other.X, Y + Other.Y };
    }

    constexpr SVec2<T>& operator+=(const SVec2<T>& Other)
    {
        X += Other.X;
        Y += Other.Y;
        return *this;
    }

    constexpr SVec2<T> operator-() const
    {
        return { -X, -Y };
    }

    constexpr SVec2<T> operator*(const T A) const
    {
        return { X * A, Y * A };
    }

    constexpr SVec2<T> operator/(const T A) const
    {
        return { X / A, Y / A };
    }

    constexpr bool operator==(const SVec2<T>& Other) const
    {
        return X == Other.X && Y == Other.Y;
    }

    constexpr bool operator!=(const SVec2<T>& Other) const
    {
        return X != Other.X || Y != Other.Y;
    }

    constexpr SVec2<T> Swapped() const
    {
        return { Y, X };
    }

    //    [[nodiscard]] SVec2<T> Cross(const SVec3<T>& Other) const
    //    {
    //        SVec3<T> Result{};
    //        Result.X = Y * Other.Z - Z * Other.Y;
    //        Result.Y = Z * Other.X - X * Other.Z;
    //        Result.Z = X * Other.Y - Y * Other.X;
    //        return Result;
    //    }
};

template <typename T>
struct SVec3
{
    T X{};
    T Y{};
    T Z{};

    constexpr SVec3() = default;

    constexpr SVec3(T InX, T InY, T InZ)
        : X(InX), Y(InY), Z(InZ){};

    explicit constexpr SVec3(SVec2<T> Vec2)
    {
        X = Vec2.X;
        Y = Vec2.Y;
    }

    explicit constexpr SVec3(T InValue)
        : X(InValue), Y(InValue), Z(InValue){};

    constexpr SVec3<T> operator+(const SVec3<T>& Other) const
    {
        return { X + Other.X, Y + Other.Y, Z + Other.Z };
    }

    constexpr SVec3<T> operator-(const SVec3<T>& Other) const
    {
        return { X - Other.X, Y - Other.Y, Z - Other.Z };
    }

    constexpr SVec3<T> operator*(const T A) const
    {
        return { X * A, Y * A, Z * A };
    }

    constexpr SVec3<T> operator/(const T A) const
    {
        return { X / A, Y / A, Z / A };
    }

    constexpr SVec3<T>& operator+=(const SVec3<T>& Other)
    {
        X += Other.X;
        Y += Other.Y;
        Z += Other.Z;
        return *this;
    }

    constexpr SVec3<T> operator-() const
    {
        return { -X, -Y, -Z };
    }

    constexpr bool operator==(const SVec3<T>& Other) const
    {
        return X == Other.X && Y == Other.Y && Z == Other.Z;
    }

    [[nodiscard]] constexpr SVec3<T> Normalized() const
    {
        SVec3<T> NormalizedVector = *this;
        float Sqr = NormalizedVector.X * NormalizedVector.X + NormalizedVector.Y * NormalizedVector.Y + NormalizedVector.Z * NormalizedVector.Z;
        return NormalizedVector * (1.0f / std::sqrt(Sqr));
    }

    [[nodiscard]] constexpr T Dot(const SVec3<T>& Other) const
    {
        return X * Other.X + Y * Other.Y + Z * Other.Z;
    }

    [[nodiscard]] constexpr SVec3<T> Cross(const SVec3<T>& Other) const
    {
        SVec3<T> Result{};
        Result.X = Y * Other.Z - Z * Other.Y;
        Result.Y = Z * Other.X - X * Other.Z;
        Result.Z = X * Other.Y - Y * Other.X;
        return Result;
    }

    [[nodiscard]] static inline constexpr SVec3<T> Mix(const SVec3<T>& A, const SVec3<T>& B, const T Alpha)
    {
        SVec3<T> Result = (B - A) * Alpha;
        return A + Result;
    }
};

template <typename T>
struct SVec4
{
    T X{};
    T Y{};
    T Z{};
    T W{};

    constexpr SVec4<T>() = default;

    constexpr SVec4<T>(T InX, T InY, T InZ, T InW)
        : X(InX), Y(InY), Z(InZ), W(InW)
    {
    }

    explicit constexpr SVec4(T InValue)
        : X(InValue), Y(InValue), Z(InValue), W(InValue){};

    constexpr SVec4<T>(const SVec3<T> FromVector3, T InW)
        : X(FromVector3.X), Y(FromVector3.Y), Z(FromVector3.Z), W(InW)
    {
    }

    constexpr SVec4<T> operator+(const SVec4<T>& Other) const
    {
        return { X + Other.X, Y + Other.Y, Z + Other.Z, W + Other.W };
    }

    constexpr SVec4<T> operator*(const float A) const
    {
        return { X * A, Y * A, Z * A, W * A };
    }

    constexpr SVec4<T>& operator+=(const SVec4<T>& Other)
    {
        X += Other.X;
        Y += Other.Y;
        Z += Other.Z;
        W += Other.W;
        return *this;
    }

    constexpr SVec4<T> operator-() const
    {
        return { -X, -Y, -Z, -W };
    }
};

using UVec2 = SVec2<float>;
using UVec2Int = SVec2<int>;
using UVec3 = SVec3<float>;
using UVec3Int = SVec3<int>;
using UVec2Size = SVec2<size_t>;
using UVec3Size = SVec3<size_t>;
using UVec4 = SVec4<float>;

template <typename T>
struct SRect
{
    SVec2<T> Min{};
    SVec2<T> Max{};

    constexpr SRect() = default;

    template <typename AnotherT>
    explicit constexpr SRect(SRect<AnotherT> Another)
    {
        Min.X = static_cast<T>(Another.Min.X);
        Min.Y = static_cast<T>(Another.Min.Y);
        Max.X = static_cast<T>(Another.Max.X);
        Max.Y = static_cast<T>(Another.Max.Y);
    }

    explicit constexpr SRect(T X, T Y, T Z, T W)
    {
        Min.X = X;
        Min.Y = Y;
        Max.X = Z;
        Max.Y = W;
    }

    explicit constexpr SRect(const SVec2<T>& A, const SVec2<T>& B)
    {
        Min = A;
        Max = B;
    }

    static constexpr SRect FromTwo(const SVec2<T>& A, const SVec2<T>& B)
    {
        SRect NewRect;

        auto MinX = std::min(A.X, B.X);
        auto MinY = std::min(A.Y, B.Y);

        auto MaxX = std::max(A.X, B.X);
        auto MaxY = std::max(A.Y, B.Y);

        NewRect.Min = { MinX, MinY };
        NewRect.Max = { MaxX, MaxY };

        return NewRect;
    }

    constexpr SRect<T> operator+(const SRect<T>& Other) const
    {
        return SRect<T>{ Min + Other.Min, Max + Other.Max };
    }

    constexpr SRect<T> operator-(const SRect<T>& Other) const
    {
        return SRect<T>{ Min - Other.Min, Max - Other.Max };
    }

    constexpr SRect<T> operator*(const T A) const
    {
        return SRect<T>{ Min * A, Max * A };
    }
};

using URect = SRect<float>;
using URectInt = SRect<int>;

template <typename T>
struct SMat4x4
{
    UVec4 X{};
    UVec4 Y{};
    UVec4 Z{};
    UVec4 W{};

    constexpr void Translate(const SVec3<T>& Vector)
    {
        W = X * Vector.X + Y * Vector.Y + Z * Vector.Z + W;
    }

    constexpr void Rotate(const float Angle, SVec3<T> Axis = SVec3<T>{ 0, 1, 0 })
    {
        T const Cos = std::cos(Angle);
        T const Sin = std::sin(Angle);

        Axis = Axis.Normalized();
        auto Temp = Axis * (T(1) - Cos);

        SMat4x4<T> Rotate;
        Rotate.X.X = Cos + Temp.X * Axis.X;
        Rotate.X.Y = Temp.X * Axis.Y + Sin * Axis.Z;
        Rotate.X.Z = Temp.X * Axis.Z - Sin * Axis.Y;

        Rotate.Y.X = Temp.Y * Axis.X - Sin * Axis.Z;
        Rotate.Y.Y = Cos + Temp.Y * Axis.Y;
        Rotate.Y.Z = Temp.Y * Axis.Z + Sin * Axis.X;

        Rotate.Z.X = Temp.Z * Axis.X + Sin * Axis.Y;
        Rotate.Z.Y = Temp.Z * Axis.Y - Sin * Axis.X;
        Rotate.Z.Z = Cos + Temp.Z * Axis.Z;

        auto Current = *this;
        X = Current.X * Rotate.X.X + Current.Y * Rotate.X.Y + Current.Z * Rotate.X.Z;
        Y = Current.X * Rotate.Y.X + Current.Y * Rotate.Y.Y + Current.Z * Rotate.Y.Z;
        Z = Current.X * Rotate.Z.X + Current.Y * Rotate.Z.Y + Current.Z * Rotate.Z.Z;
    }

    static inline constexpr SMat4x4<T> One()
    {
        return { { T(1), T(1), T(1), T(1) },
            { T(1), T(1), T(1), T(1) },
            { T(1), T(1), T(1), T(1) },
            { T(1), T(1), T(1), T(1) } };
    }

    static inline constexpr SMat4x4<T> Identity()
    {
        return { { T(1), T(0), T(0), T(0) },
            { T(0), T(1), T(0), T(0) },
            { T(0), T(0), T(1), T(0) },
            { T(0), T(0), T(0), T(1) } };
    }

    static inline constexpr SMat4x4<T> LookAtRH(const SVec3<T>& Eye, const SVec3<T>& Center, const SVec3<T>& Up)
    {
        SVec3<T> const F((Center - Eye).Normalized());
        SVec3<T> const S = F.Cross(Up).Normalized();
        SVec3<T> const U(S.Cross(F));

        SMat4x4<T> Result = Identity();
        Result.X.X = S.X;
        Result.Y.X = S.Y;
        Result.Z.X = S.Z;
        Result.X.Y = U.X;
        Result.Y.Y = U.Y;
        Result.Z.Y = U.Z;
        Result.X.Z = -F.X;
        Result.Y.Z = -F.Y;
        Result.Z.Z = -F.Z;
        Result.W.X = -S.Dot(Eye);
        Result.W.Y = -U.Dot(Eye);
        Result.W.Z = F.Dot(Eye);
        return Result;
    }
};

using UMat4x4 = SMat4x4<float>;

namespace Math
{
    constexpr float PI = 3.141592653589793f;
    constexpr float HalfPI = PI / 2.0f;

    constexpr float Radians(float Degrees)
    {
        return Degrees * 0.01745329251994329576923690768489f;
    }

    template <typename T, typename AlphaT>
    constexpr T Mix(T A, T B, AlphaT F)
    {
        return A * (1.0 - F) + (B * F);
    }

    // template <typename T>
    // constexpr T Mix(T A, T B, T F)
    // {
    //     return A * (1.0 - F) + (B * F);
    // }

    template <typename T>
    constexpr T EaseInOutCirc(T Value)
    {
        if (Value < 0.5)
        {
            return (1 - sqrt(1 - 2 * Value)) * 0.5;
        }
        else
        {
            return (1 + sqrt(2 * Value - 1)) * 0.5;
        }
    }

    template <typename T>
    constexpr T EaseInBack(T Value)
    {
        return Value * Value * (2.70158 * Value - 1.70158);
    }

    template <typename T>
    constexpr T EaseOutQuad(T Value)
    {
        return 1 - (1 - Value) * (1 - Value);
    }

    template <typename T>
    constexpr T Step(T Edge, T Value)
    {
        return Edge < Value ? 0.0 : 1.0;
    }

    // template <typename T>
    // constexpr T InterpToConstant(T from, T to, T deltaSpeed)
    // {
    //     if (to - from > 0)
    //     {
    //         return std::min(to, from + deltaSpeed);
    //     }
    //     else
    //     {
    //         return std::max(to, from - deltaSpeed);
    //     }
    // }
    template <typename T>
    constexpr auto InterpToConstant(T Current, T Target, float DeltaTime, float InterpSpeed)
    {
        if (InterpSpeed <= 0.0f)
        {
            return static_cast<T>(Target);
        }
        const auto Dist = std::abs(Target - Current);
        if (Dist * Dist < 0.00001f)
        {
            return static_cast<T>(Target);
        }
        const auto Step = DeltaTime * InterpSpeed;
        return Current + std::clamp((Target - Current), -Step, Step);
    }
}
