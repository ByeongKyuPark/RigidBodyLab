#include "math/Matrix3.h"
#include <iostream>

using namespace Math;

Matrix3::Matrix3(float diagonal) {
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            entries[j][i] = (i == j) ? diagonal : 0.0f;
        }
    }
}

Matrix3::Matrix3(float diagonal1, float diagonal2, float diagonal3) {
    entries[0][0] = diagonal1;
    entries[1][0] = 0.0f;
    entries[2][0] = 0.0f;

    entries[0][1] = 0.0f;
    entries[1][1] = diagonal2;
    entries[2][1] = 0.0f;

    entries[0][2] = 0.0f;
    entries[1][2] = 0.0f;
    entries[2][2] = diagonal3;
}

Matrix3::Matrix3(float e1, float e2, float e3, float e4, float e5, float e6, float e7, float e8, float e9) {
    entries[0][0] = e1;
    entries[1][0] = e2;
    entries[2][0] = e3;

    entries[0][1] = e4;
    entries[1][1] = e5;
    entries[2][1] = e6;

    entries[0][2] = e7;
    entries[1][2] = e8;
    entries[2][2] = e9;
}

Matrix3 Matrix3::Transpose() const {
    Matrix3 result;
    for (int i{}; i < 3; ++i) {
        for (int j{}; j < 3; ++j) {
            result.entries[i][j] = entries[j][i];
        }
    }
    return result;
}

Matrix3 Matrix3::Inverse() const
{
    float determinant =
        entries[0][0] * (entries[1][1] * entries[2][2] - entries[1][2] * entries[2][1])
        - entries[0][1] * (entries[1][0] * entries[2][2] - entries[1][2] * entries[2][0])
        + entries[0][2] * (entries[1][0] * entries[2][1] - entries[1][1] * entries[2][0]);

    if (fabs(determinant) < FLT_EPSILON) {
        throw std::runtime_error("Matrix3::inverse()::singular matrix");
    }

    determinant = 1.0f / determinant; // multiplication is cheaper than division

    Matrix3 result;

    result.entries[0][0] = determinant * (entries[1][1] * entries[2][2] - entries[1][2] * entries[2][1]);
    result.entries[1][0] = determinant * (entries[1][2] * entries[2][0] - entries[1][0] * entries[2][2]);
    result.entries[2][0] = determinant * (entries[1][0] * entries[2][1] - entries[1][1] * entries[2][0]);
    result.entries[0][1] = determinant * (entries[0][2] * entries[2][1] - entries[0][1] * entries[2][2]);
    result.entries[1][1] = determinant * (entries[0][0] * entries[2][2] - entries[0][2] * entries[2][0]);
    result.entries[2][1] = determinant * (entries[0][1] * entries[2][0] - entries[0][0] * entries[2][1]);
    result.entries[0][2] = determinant * (entries[0][1] * entries[1][2] - entries[0][2] * entries[1][1]);
    result.entries[1][2] = determinant * (entries[0][2] * entries[1][0] - entries[0][0] * entries[1][2]);
    result.entries[2][2] = determinant * (entries[0][0] * entries[1][1] - entries[0][1] * entries[1][0]);

    return result;
}


Matrix3 Matrix3::operator+(const Matrix3& other) const {
    Matrix3 result;

    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            result.entries[j][i] = entries[j][i] + other.entries[j][i];
        }
    }
    return result;
}

Matrix3& Matrix3::operator+=(const Matrix3& other) {
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            entries[j][i] += other.entries[j][i];
        }
    }
    return *this;
}

Matrix3 Matrix3::operator-(const Matrix3& other) const {
    Matrix3 result;

    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            result.entries[j][i] = entries[j][i] - other.entries[j][i];
        }
    }

    return result;
}

Matrix3& Matrix3::operator-=(const Matrix3& other) {
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            entries[j][i] -= other.entries[j][i];
        }
    }
    return *this;
}

Matrix3 Matrix3::operator*(const Matrix3& other) const {
    Matrix3 result;

    //zero out
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            result.entries[j][i] = 0.f;
        }
    }

    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            for (int k = 0; k < 3; ++k) {
                result.entries[j][i] += entries[k][i] * other.entries[j][k];
            }
        }
    }
    return result;
}

Matrix3& Matrix3::operator*=(const Matrix3& other) {
    Matrix3 result;

    // Initialize result matrix with zeros
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            result.entries[j][i] = 0.0f;
        }
    }

    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            for (int k = 0; k < 3; ++k) {
                result.entries[j][i] += this->entries[k][i] * other.entries[j][k];
            }
        }
    }

    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            entries[j][i] = result.entries[j][i];
        }
    }

    return *this;
}

Vector3 Matrix3::operator*(const Vector3& vec) const {
    return Vector3(
        entries[0][0] * vec.x + entries[0][1] * vec.y + entries[0][2] * vec.z,
        entries[1][0] * vec.x + entries[1][1] * vec.y + entries[1][2] * vec.z,
        entries[2][0] * vec.x + entries[2][1] * vec.y + entries[2][2] * vec.z
    );
}

Matrix3 Matrix3::operator*(const float value) const
{
    Matrix3 result;

    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            result.entries[j][i] *= entries[j][i] * value;
        }
    }
    return result;
}

Matrix3& Matrix3::operator*=(const float value) {
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            entries[j][i] *= value;
        }
    }
    return *this;
}

Matrix3& Matrix3::operator=(const Matrix3& other)
{
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            entries[j][i] = other.entries[j][i];
        }
    }
    return *this;
}

bool Matrix3::operator==(const Matrix3& other) const {
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            if (entries[j][i] != other.entries[j][i]) {
                return false;
            }
        }
    }
    return true;
}

bool Matrix3::operator!=(const Matrix3& other) const {
    return !(*this == other);
}

float Matrix3::operator[](int idx) const {
    if (idx < 0 || idx > 8) {
        throw std::out_of_range("Index out of bounds for Matrix3");
    }
    return entries[idx % 3][idx / 3];
}

float& Matrix3::operator[](int idx) {
    if (idx < 0 || idx > 8) {
        throw std::out_of_range("Index out of bounds for Matrix3");
    }
    return entries[idx % 3][idx / 3];
}

void Matrix3::SetDiagonal(float value) {
    entries[0][0] = entries[1][1] = entries[2][2] = value;
}
