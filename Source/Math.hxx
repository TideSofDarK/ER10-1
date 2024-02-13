#pragma once

#include <cmath>
#include <algorithm>
#include <type_traits>

template <typename T>
struct TVec2
{
    using Type = T;

    T X{};
    T Y{};

    constexpr TVec2() = default;

    constexpr TVec2(T InX, T InY)
        : X(InX), Y(InY){};

    explicit constexpr TVec2(T InValue)
        : X(InValue), Y(InValue){};

    template <typename AnotherT, typename = typename std::enable_if<std::is_same<T, AnotherT>::value == false>::type>
    explicit constexpr TVec2(const TVec2<AnotherT>& Another)
    {
        X = static_cast<T>(Another.X);
        Y = static_cast<T>(Another.Y);
    }

    constexpr TVec2<T> operator-(const TVec2<T>& Other) const
    {
        return { X - Other.X, Y - Other.Y };
    }

    constexpr TVec2<T> operator+(const TVec2<T>& Other) const
    {
        return { X + Other.X, Y + Other.Y };
    }

    constexpr TVec2<T>& operator+=(const TVec2<T>& Other)
    {
        X += Other.X;
        Y += Other.Y;
        return *this;
    }

    constexpr TVec2<T> operator-() const
    {
        return { -X, -Y };
    }

    constexpr TVec2<T> operator*(const T A) const
    {
        return { X * A, Y * A };
    }

    constexpr TVec2<T>& operator*=(const TVec2<T>& Other)
    {
        X *= Other.X;
        Y *= Other.Y;
        return *this;
    }

    constexpr TVec2<T>& operator*=(const T Other)
    {
        X *= Other;
        Y *= Other;
        return *this;
    }

    constexpr TVec2<T> operator/(const T A) const
    {
        return { X / A, Y / A };
    }

    template<typename OtherT>
    constexpr TVec2<T>& operator=(const TVec2<OtherT>& Other)
    {
        X = static_cast<T>(Other.X);
        Y = static_cast<T>(Other.Y);
        return *this;
    }

    constexpr bool operator==(const TVec2<T>& Other) const
    {
        return X == Other.X && Y == Other.Y;
    }

    constexpr bool operator!=(const TVec2<T>& Other) const
    {
        return X != Other.X || Y != Other.Y;
    }

    constexpr TVec2<T> Swapped() const
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
struct TVec3
{
    T X{};
    T Y{};
    T Z{};

    constexpr TVec3() = default;

    constexpr TVec3(T InX, T InY, T InZ)
        : X(InX), Y(InY), Z(InZ){};

    explicit constexpr TVec3(TVec2<T> Vec2)
    {
        X = Vec2.X;
        Y = Vec2.Y;
    }

    explicit constexpr TVec3(T InValue)
        : X(InValue), Y(InValue), Z(InValue){};

    constexpr TVec3<T> operator+(const TVec3<T>& Other) const
    {
        return { X + Other.X, Y + Other.Y, Z + Other.Z };
    }

    constexpr TVec3<T> operator-(const TVec3<T>& Other) const
    {
        return { X - Other.X, Y - Other.Y, Z - Other.Z };
    }

    constexpr TVec3<T> operator*(const T A) const
    {
        return { X * A, Y * A, Z * A };
    }

    constexpr TVec3<T> operator/(const T A) const
    {
        return { X / A, Y / A, Z / A };
    }

    constexpr TVec3<T>& operator+=(const TVec3<T>& Other)
    {
        X += Other.X;
        Y += Other.Y;
        Z += Other.Z;
        return *this;
    }

    constexpr TVec3<T> operator-() const
    {
        return { -X, -Y, -Z };
    }

    constexpr bool operator==(const TVec3<T>& Other) const
    {
        return X == Other.X && Y == Other.Y && Z == Other.Z;
    }

    [[nodiscard]] constexpr TVec3<T> Normalized() const
    {
        TVec3<T> NormalizedVector = *this;
        float Sqr = NormalizedVector.X * NormalizedVector.X + NormalizedVector.Y * NormalizedVector.Y + NormalizedVector.Z * NormalizedVector.Z;
        return NormalizedVector * (1.0f / std::sqrt(Sqr));
    }

    [[nodiscard]] constexpr T Dot(const TVec3<T>& Other) const
    {
        return X * Other.X + Y * Other.Y + Z * Other.Z;
    }

    [[nodiscard]] constexpr TVec3<T> Cross(const TVec3<T>& Other) const
    {
        TVec3<T> Result{};
        Result.X = Y * Other.Z - Z * Other.Y;
        Result.Y = Z * Other.X - X * Other.Z;
        Result.Z = X * Other.Y - Y * Other.X;
        return Result;
    }

    [[nodiscard]] static inline constexpr TVec3<T> Mix(const TVec3<T>& A, const TVec3<T>& B, const T Alpha)
    {
        TVec3<T> Result = (B - A) * Alpha;
        return A + Result;
    }
};

template <typename T>
struct TVec4
{
    T X{};
    T Y{};
    T Z{};
    T W{};

    constexpr TVec4<T>() = default;

    constexpr TVec4<T>(T InX, T InY, T InZ, T InW)
        : X(InX), Y(InY), Z(InZ), W(InW)
    {
    }

    explicit constexpr TVec4(T InValue)
        : X(InValue), Y(InValue), Z(InValue), W(InValue){};

    constexpr TVec4<T>(const TVec3<T> FromVector3, T InW)
        : X(FromVector3.X), Y(FromVector3.Y), Z(FromVector3.Z), W(InW)
    {
    }

    constexpr TVec4<T> operator+(const TVec4<T>& Other) const
    {
        return { X + Other.X, Y + Other.Y, Z + Other.Z, W + Other.W };
    }

    constexpr TVec4<T> operator*(const float A) const
    {
        return { X * A, Y * A, Z * A, W * A };
    }

    constexpr TVec4<T>& operator+=(const TVec4<T>& Other)
    {
        X += Other.X;
        Y += Other.Y;
        Z += Other.Z;
        W += Other.W;
        return *this;
    }

    constexpr TVec4<T> operator-() const
    {
        return { -X, -Y, -Z, -W };
    }
};

using SVec2 = TVec2<float>;
using SVec2Int = TVec2<int>;
using SVec3 = TVec3<float>;
using SVec3Int = TVec3<int>;
using SVec2Size = TVec2<size_t>;
using SVec3Size = TVec3<size_t>;
using SVec4 = TVec4<float>;

template <typename T>
struct TRect
{
    TVec2<T> Min{};
    TVec2<T> Max{};

    constexpr TRect() = default;

    template <typename AnotherT>
    explicit constexpr TRect(TRect<AnotherT> Another)
    {
        Min.X = static_cast<T>(Another.Min.X);
        Min.Y = static_cast<T>(Another.Min.Y);
        Max.X = static_cast<T>(Another.Max.X);
        Max.Y = static_cast<T>(Another.Max.Y);
    }

    explicit constexpr TRect(T X, T Y, T Z, T W)
    {
        Min.X = X;
        Min.Y = Y;
        Max.X = Z;
        Max.Y = W;
    }

    explicit constexpr TRect(const TVec2<T>& A, const TVec2<T>& B)
    {
        Min = A;
        Max = B;
    }

    static constexpr TRect FromTwo(const TVec2<T>& A, const TVec2<T>& B)
    {
        TRect NewRect;

        auto MinX = std::min(A.X, B.X);
        auto MinY = std::min(A.Y, B.Y);

        auto MaxX = std::max(A.X, B.X);
        auto MaxY = std::max(A.Y, B.Y);

        NewRect.Min = { MinX, MinY };
        NewRect.Max = { MaxX, MaxY };

        return NewRect;
    }

    constexpr TRect<T> operator+(const TRect<T>& Other) const
    {
        return TRect<T>{ Min + Other.Min, Max + Other.Max };
    }

    constexpr TRect<T> operator-(const TRect<T>& Other) const
    {
        return TRect<T>{ Min - Other.Min, Max - Other.Max };
    }

    constexpr TRect<T> operator*(const T A) const
    {
        return TRect<T>{ Min * A, Max * A };
    }
};

using SRect = TRect<float>;
using SRectInt = TRect<int>;

template <typename T>
struct TMat2x2
{
    SVec2 X{};
    SVec2 Y{};

    constexpr TMat2x2() = default;
};

template <typename T>
struct TMat4x4
{
    SVec4 X{};
    SVec4 Y{};
    SVec4 Z{};
    SVec4 W{};

    constexpr void Translate(const TVec3<T>& Vector)
    {
        W = X * Vector.X + Y * Vector.Y + Z * Vector.Z + W;
    }

    constexpr void Rotate(const float Angle, TVec3<T> Axis = TVec3<T>{ 0, 1, 0 })
    {
        T const Cos = std::cos(Angle);
        T const Sin = std::sin(Angle);

        Axis = Axis.Normalized();
        auto Temp = Axis * (T(1) - Cos);

        TMat4x4<T> Rotate;
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

    static inline constexpr TMat4x4<T> One()
    {
        return { { T(1), T(1), T(1), T(1) },
            { T(1), T(1), T(1), T(1) },
            { T(1), T(1), T(1), T(1) },
            { T(1), T(1), T(1), T(1) } };
    }

    static inline constexpr TMat4x4<T> Identity()
    {
        return { { T(1), T(0), T(0), T(0) },
            { T(0), T(1), T(0), T(0) },
            { T(0), T(0), T(1), T(0) },
            { T(0), T(0), T(0), T(1) } };
    }

    static inline constexpr TMat4x4<T> LookAtRH(const TVec3<T>& Eye, const TVec3<T>& Center, const TVec3<T>& Up)
    {
        TVec3<T> const F((Center - Eye).Normalized());
        TVec3<T> const S = F.Cross(Up).Normalized();
        TVec3<T> const U(S.Cross(F));

        TMat4x4<T> Result = Identity();
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

using SMat4x4 = TMat4x4<float>;

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
