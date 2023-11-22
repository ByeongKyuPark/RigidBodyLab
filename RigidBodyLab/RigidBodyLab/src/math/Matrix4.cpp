#include "math/Matrix3.h"
#include "math/Matrix4.h"
#include <iostream>
#include <immintrin.h>

using namespace Math;

Matrix4::Matrix4(float value) {
    columns[0] = _mm_set_ps(0.0f, 0.0f, 0.0f, value);
    columns[1] = _mm_set_ps(0.0f, 0.0f, value, 0.0f);
    columns[2] = _mm_set_ps(0.0f, value, 0.0f, 0.0f);
    columns[3] = _mm_set_ps(value, 0.0f, 0.0f, 0.0f);
}

Matrix4::Matrix4(float v1, float v2, float v3, float v4) {
    columns[0] = _mm_set_ps(0.0f, 0.0f, 0.0f, v1);
    columns[1] = _mm_set_ps(0.0f, 0.0f, v2, 0.0f);
    columns[2] = _mm_set_ps(0.0f, v3, 0.0f, 0.0f);
    columns[3] = _mm_set_ps(v4, 0.0f, 0.0f, 0.0f);
}

Matrix4::Matrix4(float e00, float e01, float e02, float e03,
    float e10, float e11, float e12, float e13,
    float e20, float e21, float e22, float e23,
    float e30, float e31, float e32, float e33) {
    columns[0] = _mm_set_ps(e30, e20, e10, e00);
    columns[1] = _mm_set_ps(e31, e21, e11, e01);
    columns[2] = _mm_set_ps(e32, e22, e12, e02);
    columns[3] = _mm_set_ps(e33, e23, e13, e03);
}
Matrix4 Matrix4::Transpose() const {
    Matrix4 result;

    __m128 tmp3, tmp2, tmp1, tmp0;
    tmp0 = _mm_unpacklo_ps(columns[0], columns[1]);
    tmp2 = _mm_unpacklo_ps(columns[2], columns[3]);
    tmp1 = _mm_unpackhi_ps(columns[0], columns[1]);
    tmp3 = _mm_unpackhi_ps(columns[2], columns[3]);
    result.columns[0] = _mm_movelh_ps(tmp0, tmp2);
    result.columns[1] = _mm_movehl_ps(tmp2, tmp0);
    result.columns[2] = _mm_movelh_ps(tmp1, tmp3);
    result.columns[3] = _mm_movehl_ps(tmp3, tmp1);

    return result;
}
Matrix4 Matrix4::Inverse() const
{
    float inv[16];

    // Local variables for often-used matrix elements
    float m0 = operator[](0), m1 = operator[](1), m2 = operator[](2), m3 = operator[](3),
        m4 = operator[](4), m5 = operator[](5), m6 = operator[](6), m7 = operator[](7),
        m8 = operator[](8), m9 = operator[](9), m10 = operator[](10), m11 = operator[](11),
        m12 = operator[](12), m13 = operator[](13), m14 = operator[](14), m15 = operator[](15);

    // Compute the Inverse using adjugate and determinant
    inv[0] = m5 * (m10 * m15 - m11 * m14) + m9 * (m7 * m14 - m6 * m15) + m13 * (m6 * m11 - m7 * m10);
    inv[1] = m1 * (m11 * m14 - m10 * m15) + m9 * (m2 * m15 - m3 * m14) + m13 * (m3 * m10 - m2 * m11);
    inv[2] = m1 * (m6 * m15 - m7 * m14) + m5 * (m3 * m14 - m2 * m15) + m13 * (m2 * m7 - m3 * m6);
    inv[3] = m1 * (m7 * m10 - m6 * m11) + m5 * (m2 * m11 - m3 * m10) + m9 * (m3 * m6 - m2 * m7);

    inv[4] = m4 * (m11 * m14 - m10 * m15) + m8 * (m6 * m15 - m7 * m14) + m12 * (m7 * m10 - m6 * m11);
    inv[5] = m0 * (m10 * m15 - m11 * m14) + m8 * (m3 * m14 - m2 * m15) + m12 * (m2 * m11 - m3 * m10);
    inv[6] = m0 * (m7 * m14 - m6 * m15) + m4 * (m2 * m15 - m3 * m14) + m12 * (m3 * m6 - m2 * m7);
    inv[7] = m0 * (m6 * m11 - m7 * m10) + m4 * (m3 * m10 - m2 * m11) + m8 * (m2 * m7 - m3 * m6);

    inv[8] = m4 * (m9 * m15 - m11 * m13) + m8 * (m7 * m13 - m5 * m15) + m12 * (m5 * m11 - m7 * m9);
    inv[9] = m0 * (m11 * m13 - m9 * m15) + m8 * (m1 * m15 - m3 * m13) + m12 * (m3 * m9 - m1 * m11);
    inv[10] = m0 * (m5 * m15 - m7 * m13) + m4 * (m3 * m13 - m1 * m15) + m12 * (m1 * m7 - m3 * m5);
    inv[11] = m0 * (m7 * m9 - m5 * m11) + m4 * (m1 * m11 - m3 * m9) + m8 * (m3 * m5 - m1 * m7);

    inv[12] = m4 * (m10 * m13 - m9 * m14) + m8 * (m5 * m14 - m6 * m13) + m12 * (m6 * m9 - m5 * m10);
    inv[13] = m0 * (m9 * m14 - m10 * m13) + m8 * (m2 * m13 - m1 * m14) + m12 * (m1 * m10 - m2 * m9);
    inv[14] = m0 * (m6 * m13 - m5 * m14) + m4 * (m1 * m14 - m2 * m13) + m12 * (m2 * m5 - m1 * m6);
    inv[15] = m0 * (m5 * m10 - m6 * m9) + m4 * (m2 * m9 - m1 * m10) + m8 * (m1 * m6 - m2 * m5);

    // Compute the determinant using the adjugate formula
    float determinant = m0 * inv[0] + m1 * inv[4] + m2 * inv[8] + m3 * inv[12];

    if (fabs(determinant) < 1e-6f)
    {
        std::cerr << "MATRIX4::singular, inverse does not exist." << std::endl;
        return *this;
    }

    float invDet = 1.f / determinant;
    Matrix4 result;

    for (int i{}; i < 16; i++) {
        result[i] = inv[i] * invDet;
    }

    return result;
}

Matrix4 Matrix4::operator+(const Matrix4& other) const {
    Matrix4 result;

    for (int i{}; i < 4; ++i) {
        result.columns[i] = _mm_add_ps(columns[i], other.columns[i]);
    }
    return result;
}

Matrix4& Matrix4::operator+=(const Matrix4& other)
{
    for (int i{}; i < 4; ++i) {
        columns[i] = _mm_add_ps(columns[i], other.columns[i]);
    }
    return *this;
}

Matrix4 Matrix4::operator-(const Matrix4& other) const
{
    Matrix4 result;

    for (int i{}; i < 4; ++i) {
        result.columns[i] = _mm_sub_ps(columns[i], other.columns[i]);
    }

    return result;
}

Matrix4& Matrix4::operator-=(const Matrix4& other)
{
    for (int i{}; i < 4; ++i) {
        columns[i] = _mm_sub_ps(columns[i], other.columns[i]);
    }
    return *this;
}

Matrix4 Matrix4::operator*(const Matrix4& other) const {
    Matrix4 result;

    for (int i{}; i < 4; ++i) { // columns (in the result)
        for (int j{}; j < 4; ++j) { // rows (in the result)

            __m128 a_row = _mm_set_ps(columns[3].m128_f32[i], columns[2].m128_f32[i], columns[1].m128_f32[i], columns[0].m128_f32[i]);
            __m128 prod = _mm_mul_ps(a_row, other.columns[j]);

            prod = _mm_hadd_ps(prod, prod);  // Horizontal add
            prod = _mm_hadd_ps(prod, prod);

            // Flip the result both vertically and horizontally
            // Given that the result is flipped both vertically and horizontally, I've adjust the way the result is stored.
            result.columns[3 - j].m128_f32[3 - i] = _mm_cvtss_f32(prod);
        }
    }

    return result;
}





Matrix4& Matrix4::operator*=(const Matrix4& other) {
    Matrix4 temp = (*this) * other;
    this->columns = std::move(temp.columns);
    return *this;
}


Vector3 Matrix4::operator*(const Vector3& vec) const {
    float x = columns[0].m128_f32[0] * vec.x + columns[1].m128_f32[0] * vec.y + columns[2].m128_f32[0] * vec.z + columns[3].m128_f32[0];
    float y = columns[0].m128_f32[1] * vec.x + columns[1].m128_f32[1] * vec.y + columns[2].m128_f32[1] * vec.z + columns[3].m128_f32[1];
    float z = columns[0].m128_f32[2] * vec.x + columns[1].m128_f32[2] * vec.y + columns[2].m128_f32[2] * vec.z + columns[3].m128_f32[2];
    float w = columns[0].m128_f32[3] * vec.x + columns[1].m128_f32[3] * vec.y + columns[2].m128_f32[3] * vec.z + columns[3].m128_f32[3];

    if (w != 1.f && w != 0.f) {
        x /= w;
        y /= w;
        z /= w;
    }

    return Vector3(x, y, z);
}

Matrix4 Matrix4::operator*(const float value) const
{
    Matrix4 result;

    __m128 scalar = _mm_set1_ps(value);

    for (int i{}; i < 4; ++i) {
        result.columns[i] = _mm_mul_ps(columns[i], scalar);
    }

    return result;
}

Matrix4& Matrix4::operator*=(const float value)
{
    __m128 scalar = _mm_set1_ps(value);

    for (int i{}; i < 4; ++i) {
        columns[i] = _mm_mul_ps(columns[i], scalar);
    }

    return *this;
}

float Matrix4::operator[](int idx) const {
    if (idx < 0 || idx > 15) {
        throw std::out_of_range("Index out of bounds for Matrix4");
    }
    return reinterpret_cast<const float*>(&columns[idx / 4])[idx % 4];
}

float& Matrix4::operator[](int idx) {
    if (idx < 0 || idx > 15) {
        throw std::out_of_range("Index out of bounds for Matrix4");
    }
    return reinterpret_cast<float*>(&columns[idx / 4])[idx % 4];
}

Matrix4& Matrix4::operator=(const Matrix4& other) {
    if (this != &other) {  // Self-assignment check
        for (int i{}; i < 4; ++i) {
            columns[i] = other.columns[i];
        }
    }
    return *this;
}

bool Matrix4::operator==(const Matrix4& other) const {
    for (int i{}; i < 4; ++i) {
        __m128 col1 = columns[i];
        __m128 col2 = other.columns[i];

        //compare, -1 if not the same value.
        __m128 result = _mm_cmpeq_ps(col1, col2);

        if (_mm_movemask_ps(result) != 0xF) {  // 0xF = 1111 = -1
            return false;
        }
    }
    return true;
}


bool Matrix4::operator!=(const Matrix4& other) const
{
    return !(*this == other);
}

// Get element at (row, column)
float Math::Matrix4::Get(int row, int column) const {
    if (row < 0 || row > 3 || column < 0 || column > 3) {
        throw std::out_of_range("Index out of bounds for Matrix4");
    }
    return columns[row].m128_f32[column];
}


// Set element at (row, column)
void Math::Matrix4::Set(int row, int column, float value) {
    if (row < 0 || row > 3 || column < 0 || column > 3) {
        throw std::out_of_range("Index out of bounds for Matrix4");
    }
    columns[row].m128_f32[column] = value;
}

Matrix3 Matrix4::Extract3x3Matrix() const {
    Matrix3 result;

    float* matrix3Data = &result.entries[0][0];
    for (int col{}; col < 3; ++col) {
        float tmp[4];
        _mm_storeu_ps(tmp, columns[col]);

        for (int row{}; row < 3; ++row) {
            matrix3Data[col * 3 + row] = tmp[row];
        }
    }

    return result;
}


glm::mat4 Matrix4::ConvertToGLM() const noexcept {
    glm::mat4 result;

    for (int col = 0; col < 4; ++col) {
        for (int row = 0; row < 4; ++row) {
            result[col][row] = columns[row].m128_f32[col];
        }
    }

    return result;
}

