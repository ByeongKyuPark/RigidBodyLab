#pragma once

#include "Vector3.h"
#include <array>
#include <stdexcept>

namespace Math {
    class Matrix3 {
    public:
        std::array<std::array<float, 3>, 3> entries; // Column-major order

        Matrix3(float diagonal = 1.0f);
        Matrix3(float diagonal1, float diagonal2, float diagonal3) {
            entries = { { { diagonal1, 0.0f, 0.0f },{ 0.0f, diagonal2, 0.0f },{ 0.0f, 0.0f, diagonal3 } } };
        }
        Matrix3(float e1, float e2, float e3, float e4, float e5, float e6, float e7, float e8, float e9) {
            entries = { { { e1, e4, e7 },{ e2, e5, e8 },{ e3, e6, e9 } } };
        }

        [[nodiscard]] Matrix3 Transpose() const;
        [[nodiscard]] Matrix3 Inverse() const;

        Matrix3 operator+(const Matrix3& other) const;
        Matrix3& operator+=(const Matrix3& other);

        Matrix3 operator-(const Matrix3& other) const;
        Matrix3& operator-=(const Matrix3& other);

        Matrix3 operator*(const Matrix3& other) const;
        Matrix3& operator*=(const Matrix3& other);

        Vector3 operator*(const Vector3& vec) const;

        Matrix3 operator*(const float value) const;
        Matrix3& operator*=(const float value);


        bool operator==(const Matrix3& other) const;
        bool operator!=(const Matrix3& other) const;

        float operator[](int idx) const;
        float& operator[](int idx);

        void SetDiagonal(float value);
    };
}
