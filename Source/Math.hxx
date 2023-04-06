#pragma once

#include <cmath>

template<typename T>
struct SVec2 {
    T X{};
    T Y{};

    SVec2<T> operator+(const SVec2<T> &Other) const {
        return {X + Other.X, Y + Other.Y};
    }

    SVec2<T> &operator+=(const SVec2<T> &Other) {
        X += Other.X;
        Y += Other.Y;
        return *this;
    }

    SVec2<T> operator-() const {
        return {-X, -Y};
    }

    SVec2<T> Swapped() const {
        return {Y, X};
    }
};

template<typename T>
struct SVec3 {
    T X{};
    T Y{};
    T Z{};

    SVec3<T> operator+(const SVec3<T> &Other) const {
        return {X + Other.X, Y + Other.Y, Z + Other.Z};
    }

    SVec3<T> operator-(const SVec3<T> &Other) const {
        return {X - Other.X, Y - Other.Y, Z - Other.Z};
    }

    SVec3<T> operator*(const float A) const {
        return {X * A, Y * A, Z * A};
    }

    SVec3<T> &operator+=(const SVec3<T> &Other) {
        X += Other.X;
        Y += Other.Y;
        Z += Other.Z;
        return *this;
    }

    SVec3<T> operator-() const {
        return {-X, -Y, -Z};
    }

    bool operator==(const SVec3<T> &Other) const {
        return X == Other.X && Y == Other.Y && Z == Other.Z;
    }

    [[nodiscard]] SVec3<T> Normalized() const {
        SVec3<T> NormalizedVector = *this;
        float Sqr = NormalizedVector.X * NormalizedVector.X + NormalizedVector.Y * NormalizedVector.Y +
                    NormalizedVector.Z * NormalizedVector.Z;
        return NormalizedVector * (1.0f / std::sqrt(Sqr));
    }

    [[nodiscard]] T Dot(const SVec3<T> &Other) const {
        return X * Other.X + Y * Other.Y + Z * Other.Z;
    }

    [[nodiscard]] SVec3<T> Cross(const SVec3<T> &Other) const {
        SVec3<T> Result{};
        Result.X = Y * Other.Z - Z * Other.Y;
        Result.Y = Z * Other.X - X * Other.Z;
        Result.Z = X * Other.Y - Y * Other.X;
        return Result;
    };

    [[nodiscard]] static inline SVec3<T> Mix(const SVec3<T> &A, const SVec3<T> &B, const T Alpha) {
        SVec3<T> Result = (B - A) * Alpha;
        return A + Result;
    };
};

template<typename T>
struct SVec4 {
    T X{};
    T Y{};
    T Z{};
    T W{};

    SVec4<T>() = default;

    SVec4<T>(T InX, T InY, T InZ, T InW) : X(InX), Y(InY), Z(InZ), W(InW) {

    }

    SVec4<T>(const SVec3<T> FromVector3, T InW) : X(FromVector3.X), Y(FromVector3.Y), Z(FromVector3.Z), W(InW) {

    }

    SVec4<T> operator+(const SVec4<T> &Other) const {
        return {X + Other.X, Y + Other.Y, Z + Other.Z, W + Other.W};
    }

    SVec4<T> operator*(const float A) const {
        return {X * A, Y * A, Z * A, W * A};
    }

    SVec4<T> &operator+=(const SVec4<T> &Other) {
        X += Other.X;
        Y += Other.Y;
        Z += Other.Z;
        W += Other.W;
        return *this;
    }

    SVec4<T> operator-() const {
        return {-X, -Y, -Z, -W};
    }
};

using UVec2 = SVec2<float>;
using UVec2Int = SVec2<int>;
using UVec3 = SVec3<float>;
using UVec3Int = SVec3<int>;
using UVec4 = SVec4<float>;

template<typename T>
struct SMat4x4 {
    UVec4 X{};
    UVec4 Y{};
    UVec4 Z{};
    UVec4 W{};

    void Translate(const SVec3<T> &Vector) {
        W = X * Vector.X + Y * Vector.Y + Z * Vector.Z + W;
    }

    void Rotate(const float Angle, SVec3<T> Axis) {
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

    static inline SMat4x4<T> One() {
        return {{T(1), T(1), T(1), T(1)},
                {T(1), T(1), T(1), T(1)},
                {T(1), T(1), T(1), T(1)},
                {T(1), T(1), T(1), T(1)}};
    }

    static inline SMat4x4<T> Identity() {
        return {{T(1), T(0), T(0), T(0)},
                {T(0), T(1), T(0), T(0)},
                {T(0), T(0), T(1), T(0)},
                {T(0), T(0), T(0), T(1)}};
    }

    static inline SMat4x4<T> LookAtRH(const SVec3<T> &Eye, const SVec3<T> &Center, const SVec3<T> &Up) {
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

namespace Math {
    constexpr float PI = 3.141592653589793f;

    constexpr float Radians(float Degrees) {
        return Degrees * 0.01745329251994329576923690768489f;
    }

//    template <typename T>
//    constexpr T Clamp(T A, T Min, T Max) {
//        return std::max(std::min(A, Max), Min);
//    }
}