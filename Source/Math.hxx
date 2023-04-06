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

    SVec2<T> &operator=(const SVec2<T> &Other) {
        X = Other.X;
        Y = Other.Y;
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

    SVec3<T> &operator=(const SVec3<T> &Other) {
        X = Other.X;
        Y = Other.Y;
        Z = Other.Z;
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

    SVec4<T> &operator=(const SVec4<T> &Other) {
        X = Other.X;
        Y = Other.Y;
        Z = Other.Z;
        W = Other.W;
        return *this;
    }

    SVec4<T> operator-() const {
        return {-X, -Y, -Z, -W};
    }

    SVec4<T> Swapped() {
        return {W, Z, Y, X};
    }
};

using UVec2 = SVec2<float>;
using UVec2Int = SVec2<int>;
using UVec3 = SVec3<float>;
using UVec3Int = SVec3<int>;
using UVec4 = SVec4<float>;

struct UMat4x4 {
    UVec4 X{};
    UVec4 Y{};
    UVec4 Z{};
    UVec4 W{};

    void Translate(const UVec3 &Vector) {
        W = X * Vector.X + Y * Vector.Y + Z * Vector.Z + W;
    }

    void Rotate(const float Angle, UVec3 Axis) {
        float const a = Angle;
        float const c = std::cos(a);
        float const s = std::sin(a);

        Axis = Axis.Normalized();
        auto Temp = Axis * (1.0f - c);

        UMat4x4 Rotate;
        Rotate.X.X = c + Temp.X * Axis.X;
        Rotate.X.Y = Temp.X * Axis.Y + s * Axis.Z;
        Rotate.X.Z = Temp.X * Axis.Z - s * Axis.Y;

        Rotate.Y.X = Temp.Y * Axis.X - s * Axis.Z;
        Rotate.Y.Y = c + Temp.Y * Axis.Y;
        Rotate.Y.Z = Temp.Y * Axis.Z + s * Axis.X;

        Rotate.Z.X = Temp.Z * Axis.X + s * Axis.Y;
        Rotate.Z.Y = Temp.Z * Axis.Y - s * Axis.X;
        Rotate.Z.Z = c + Temp.Z * Axis.Z;

        auto Current = *this;
        X = Current.X * Rotate.X.X + Current.Y * Rotate.X.Y + Current.Z * Rotate.X.Z;
        Y = Current.X * Rotate.Y.X + Current.Y * Rotate.Y.Y + Current.Z * Rotate.Y.Z;
        Z = Current.X * Rotate.Z.X + Current.Y * Rotate.Z.Y + Current.Z * Rotate.Z.Z;
    }

    static inline UMat4x4 One() {
        return {{1.0f, 1.0f, 1.0f, 1.0f},
                {1.0f, 1.0f, 1.0f, 1.0f},
                {1.0f, 1.0f, 1.0f, 1.0f},
                {1.0f, 1.0f, 1.0f, 1.0f}};
    }

    static inline UMat4x4 Identity() {
        return {{1.0f, 0.0f, 0.0f, 0.0f},
                {0.0f, 1.0f, 0.0f, 0.0f},
                {0.0f, 0.0f, 1.0f, 0.0f},
                {0.0f, 0.0f, 0.0f, 1.0f}};
    }

    static inline UMat4x4 LookAtRH(const UVec3 &Eye, const UVec3 &Center, const UVec3 &Up) {
        UVec3 const f((Center - Eye).Normalized());
        UVec3 const s = f.Cross(Up).Normalized();
        UVec3 const u(s.Cross(f));

        UMat4x4 Result = Identity();
        Result.X.X = s.X;
        Result.Y.X = s.Y;
        Result.Z.X = s.Z;
        Result.X.Y = u.X;
        Result.Y.Y = u.Y;
        Result.Z.Y = u.Z;
        Result.X.Z = -f.X;
        Result.Y.Z = -f.Y;
        Result.Z.Z = -f.Z;
        Result.W.X = -s.Dot(Eye);
        Result.W.Y = -u.Dot(Eye);
        Result.W.Z = f.Dot(Eye);
        return Result;
    }
};

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