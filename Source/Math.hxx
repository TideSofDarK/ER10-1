#pragma once

template<typename T>
struct SVec2 {
    T X{};
    T Y{};

    SVec2<T> operator+(const SVec2<T> &Other) {
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

    SVec2<T> operator-() {
        return {-X, -Y};
    }

    SVec2<T> Swapped() {
        return {Y, X};
    }
};

using UVec2 = SVec2<float>;
using UVec2Int = SVec2<int>;

namespace Math {
    constexpr float PI = 3.141592653589793f;

    constexpr float Radians(float Degrees) {
        return Degrees * 0.01745329251994329576923690768489f;
    }
}