#include"gtest/gtest.h"
#include "Vector3.h"
#include "Quaternion.h"
#include "Matrix3.h"
#include "Matrix4.h"

constexpr float EPSILON = 1e-5f;
constexpr float LOOSE_EPSILON = 1e-3f;
constexpr float PI = 3.14159265359f;

using namespace Math;

TEST(Vector3Test, DefaultConstructor) {
    Vector3 v;
    EXPECT_FLOAT_EQ(v.x, 0.0f);
    EXPECT_FLOAT_EQ(v.y, 0.0f);
    EXPECT_FLOAT_EQ(v.z, 0.0f);
}

TEST(Vector3Test, ParameterizedConstructor) {
    Vector3 v(1.0f, 2.0f, 3.0f);
    EXPECT_FLOAT_EQ(v.x, 1.0f);
    EXPECT_FLOAT_EQ(v.y, 2.0f);
    EXPECT_FLOAT_EQ(v.z, 3.0f);
}

TEST(Vector3Test, Length) {
    Vector3 v(1.0f, 2.0f, 2.0f);
    EXPECT_NEAR(v.Length(), 3.0f, EPSILON);
}

TEST(Vector3Test, LengthSquared) {
    Vector3 v(1.0f, 2.0f, 2.0f);
    EXPECT_FLOAT_EQ(v.LengthSquared(), 9.0f);
}

TEST(Vector3Test, Normalize) {
    Vector3 v(3.0f, 4.0f, 0.0f);
    v.Normalize();
    EXPECT_NEAR(v.Length(), 1.0f, EPSILON);
}

TEST(Vector3Test, DotProduct) {
    Vector3 v1(1.0f, 2.0f, 3.0f);
    Vector3 v2(4.0f, 5.0f, 6.0f);
    EXPECT_FLOAT_EQ(v1.Dot(v2), 32.0f);
}

TEST(Vector3Test, CrossProduct) {
    Vector3 v1(1.0f, 0.0f, 0.0f);
    Vector3 v2(0.0f, 1.0f, 0.0f);
    Vector3 cross = v1.Cross(v2);
    EXPECT_FLOAT_EQ(cross.x, 0.0f);
    EXPECT_FLOAT_EQ(cross.y, 0.0f);
    EXPECT_FLOAT_EQ(cross.z, 1.0f);
}

TEST(Vector3Test, AddOperator) {
    Vector3 v1(1.0f, 2.0f, 3.0f);
    Vector3 v2(4.0f, 5.0f, 6.0f);
    Vector3 result = v1 + v2;
    EXPECT_FLOAT_EQ(result.x, 5.0f);
    EXPECT_FLOAT_EQ(result.y, 7.0f);
    EXPECT_FLOAT_EQ(result.z, 9.0f);
}

TEST(Vector3Test, SubtractOperator) {
    Vector3 v1(4.0f, 5.0f, 6.0f);
    Vector3 v2(1.0f, 2.0f, 3.0f);
    Vector3 result = v1 - v2;
    EXPECT_FLOAT_EQ(result.x, 3.0f);
    EXPECT_FLOAT_EQ(result.y, 3.0f);
    EXPECT_FLOAT_EQ(result.z, 3.0f);
}

TEST(Vector3Test, MultiplyOperator) {
    Vector3 v(1.0f, 2.0f, 3.0f);
    Vector3 result = v * 2.0f;
    EXPECT_FLOAT_EQ(result.x, 2.0f);
    EXPECT_FLOAT_EQ(result.y, 4.0f);
    EXPECT_FLOAT_EQ(result.z, 6.0f);
}

TEST(Vector3Test, EqualityOperator) {
    Vector3 v1(1.0f, 2.0f, 3.0f);
    Vector3 v2(1.0f, 2.0f, 3.0f);
    EXPECT_TRUE(v1 == v2);
}

TEST(Vector3Test, InequalityOperator) {
    Vector3 v1(1.0f, 2.0f, 3.0f);
    Vector3 v2(3.0f, 2.0f, 1.0f);
    EXPECT_TRUE(v1 != v2);
}

TEST(Vector3Test, BracketOperatorRead) {
    Vector3 v(1.0f, 2.0f, 3.0f);
    EXPECT_FLOAT_EQ(v[0], 1.0f);
    EXPECT_FLOAT_EQ(v[1], 2.0f);
    EXPECT_FLOAT_EQ(v[2], 3.0f);
}

TEST(Vector3Test, BracketOperatorWrite) {
    Vector3 v;
    v[0] = 1.0f;
    v[1] = 2.0f;
    v[2] = 3.0f;
    EXPECT_FLOAT_EQ(v.x, 1.0f);
    EXPECT_FLOAT_EQ(v.y, 2.0f);
    EXPECT_FLOAT_EQ(v.z, 3.0f);
}

// Exception handling tests
TEST(Vector3Test, BracketOperatorOutOfRange) {
    Vector3 v;
    EXPECT_THROW(v[3], std::out_of_range);
}

TEST(Matrix3Test, DefaultConstructor) {
    Matrix3 m;
    for (int i{}; i < 3; ++i) {
        for (int j{}; j < 3; ++j) {
            EXPECT_FLOAT_EQ(m.entries[i][j], (i == j) ? 1.0f : 0.0f);
        }
    }
}

TEST(Matrix3Test, DiagonalConstructor) {
    Matrix3 m(2.f);
    for (int i{}; i < 3; ++i) {
        for (int j{}; j < 3; ++j) {
            EXPECT_FLOAT_EQ(m.entries[i][j], (i == j) ? 2.0f : 0.0f);
        }
    }
}

TEST(Matrix3Test, FullConstructor) {
    Matrix3 m(1, 2, 3, 4, 5, 6, 7, 8, 9);
    EXPECT_FLOAT_EQ(m.entries[0][0], 1);
    EXPECT_FLOAT_EQ(m.entries[1][0], 2);
    EXPECT_FLOAT_EQ(m.entries[2][0], 3);
    EXPECT_FLOAT_EQ(m.entries[0][1], 4);
    EXPECT_FLOAT_EQ(m.entries[1][1], 5);
    EXPECT_FLOAT_EQ(m.entries[2][1], 6);
    EXPECT_FLOAT_EQ(m.entries[0][2], 7);
    EXPECT_FLOAT_EQ(m.entries[1][2], 8);
    EXPECT_FLOAT_EQ(m.entries[2][2], 9);
}

TEST(Matrix3Test, Transpose) {
    Matrix3 m(1, 2, 3, 4, 5, 6, 7, 8, 9);
    Matrix3 mt = m.Transpose();
    EXPECT_FLOAT_EQ(mt.entries[1][0], 4);
    EXPECT_FLOAT_EQ(mt.entries[0][1], 2);
    EXPECT_FLOAT_EQ(mt.entries[2][1], 8);
    EXPECT_FLOAT_EQ(mt.entries[1][2], 6);
}

TEST(Matrix3Test, Multiplication) {
    Matrix3 m1(1, 2, 3, 4, 5, 6, 7, 8, 9);
    Matrix3 m2(9, 8, 7, 6, 5, 4, 3, 2, 1);
    Matrix3 product = m1 * m2;

    EXPECT_NEAR(product.entries[0][0], 30, EPSILON);
    EXPECT_NEAR(product.entries[0][1], 84, EPSILON);
    EXPECT_NEAR(product.entries[0][2], 138, EPSILON);
    EXPECT_NEAR(product.entries[1][0], 24, EPSILON);
    EXPECT_NEAR(product.entries[1][1], 69, EPSILON);
    EXPECT_NEAR(product.entries[1][2], 114, EPSILON);
    EXPECT_NEAR(product.entries[2][0], 18, EPSILON);
    EXPECT_NEAR(product.entries[2][1], 54, EPSILON);
    EXPECT_NEAR(product.entries[2][2], 90, EPSILON);
}

TEST(Matrix3Test, Inverse) {
    Matrix3 m(1, 2, 3, 0, 1, 4, 5, 6, 0);
    Matrix3 inverse = m.Inverse();

    EXPECT_NEAR(inverse.entries[0][0], -24, EPSILON);
    EXPECT_NEAR(inverse.entries[1][0], 18, EPSILON);
    EXPECT_NEAR(inverse.entries[2][0], 5, EPSILON);
    EXPECT_NEAR(inverse.entries[0][1], 20, EPSILON);
    EXPECT_NEAR(inverse.entries[1][1], -15, EPSILON);
    EXPECT_NEAR(inverse.entries[2][1], -4, EPSILON);
    EXPECT_NEAR(inverse.entries[0][2], -5, EPSILON);
    EXPECT_NEAR(inverse.entries[1][2], 4, EPSILON);
    EXPECT_NEAR(inverse.entries[2][2], 1, EPSILON);
}

TEST(Matrix3Test, VectorMultiplication) {
    Matrix3 m(1, 2, 3, 4, 5, 6, 7, 8, 9);
    Vector3 v(1, 2, 3);
    Vector3 result = m * v;

    EXPECT_NEAR(result.x, 30, EPSILON);
    EXPECT_NEAR(result.y, 36, EPSILON);
    EXPECT_NEAR(result.z, 42, EPSILON);
}

TEST(Matrix3Test, ScalarMultiplication) {
    Matrix3 m(1, 2, 3, 4, 5, 6, 7, 8, 9);
    float scalar = 2.f;
    Matrix3 result = m * scalar;

    EXPECT_FLOAT_EQ(result.entries[0][0], 2);
    EXPECT_FLOAT_EQ(result.entries[0][1], 8);
    EXPECT_FLOAT_EQ(result.entries[0][2], 14);
    EXPECT_FLOAT_EQ(result.entries[1][0], 4);
    EXPECT_FLOAT_EQ(result.entries[1][1], 10);
    EXPECT_FLOAT_EQ(result.entries[1][2], 16);
    EXPECT_FLOAT_EQ(result.entries[2][0], 6);
    EXPECT_FLOAT_EQ(result.entries[2][1], 12);
    EXPECT_FLOAT_EQ(result.entries[2][2], 18);
}

TEST(Matrix3_Vector3_Multiplication, MultiplyWithZeroVector) {
    Matrix3 m(1, 2, 3, 4, 5, 6, 7, 8, 9);
    Vector3 v(0, 0, 0);
    Vector3 result = m * v;

    EXPECT_NEAR(result.x, 0.0f, EPSILON);
    EXPECT_NEAR(result.y, 0.0f, EPSILON);
    EXPECT_NEAR(result.z, 0.0f, EPSILON);
}

TEST(Matrix3_Vector3_Multiplication, MultiplyWithUnitVector) {
    Matrix3 m(1, 2, 3, 4, 5, 6, 7, 8, 9);
    Vector3 v(1, 0, 0); // Unit vector along x-axis
    Vector3 result = m * v;

    EXPECT_NEAR(result.x, 1.f, EPSILON);
    EXPECT_NEAR(result.y, 2.f, EPSILON);
    EXPECT_NEAR(result.z, 3.f, EPSILON);
}

TEST(Matrix3_Vector3_Multiplication, MultiplyWithGeneralVector) {
    Matrix3 m(1, 2, 3, 4, 5, 6, 7, 8, 9);
    Vector3 v(1, 2, 3);
    Vector3 result = m * v;

    EXPECT_NEAR(result.x, 30.f, EPSILON);
    EXPECT_NEAR(result.y, 36.f, EPSILON);
    EXPECT_NEAR(result.z, 42.f, EPSILON);
}

TEST(Matrix3_Vector3_Multiplication, MultiplyWithNegativeVector) {
    Matrix3 m(1, 2, 3, 4, 5, 6, 7, 8, 9);
    Vector3 v(-1, -2, -3);
    Vector3 result = m * v;

    EXPECT_NEAR(result.x, -30.f, EPSILON);
    EXPECT_NEAR(result.y, -36.f, EPSILON);
    EXPECT_NEAR(result.z, -42.f, EPSILON);
}

TEST(Matrix3_Vector3_Multiplication, MultiplyWithInverseMatrix) {
    Matrix3 m(-3, 2, 1, 4, 5, -6, 9, -8, 7);
    Matrix3 mInv = m.Inverse();
    const Matrix3 identity(1.f);
    Matrix3 result = m * mInv;
    for (int i{}; i < 3; ++i) {
        for (int j{}; j < 3; ++j) {
            EXPECT_NEAR(result.entries[i][j], identity.entries[i][j],EPSILON);
        }
    }
    result = mInv*m;
    for (int i{}; i < 3; ++i) {
        for (int j{}; j < 3; ++j) {
            EXPECT_NEAR(result.entries[i][j], identity.entries[i][j],EPSILON);
        }
    }
}

TEST(Matrix4Test, DefaultConstructor) {
    Matrix4 m;
    // Expect the m to be an identity m
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            EXPECT_FLOAT_EQ(m.Get(i,j), (i == j) ? 1.0f : 0.0f);
        }
    }
}

TEST(Matrix4Test, GetSetWithOperatorIndex) {
    Math::Matrix4 m(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);

    // Set values using Set method
    m.Set(0, 0, 11.2f);
    m.Set(1, 1, 22.3f);
    m.Set(2, 2, 33.4f);
    m.Set(3, 3, 44.5f);

    // Test Get method against operator[]
    EXPECT_FLOAT_EQ(m.Get(0, 0), m[0]);
    EXPECT_FLOAT_EQ(m.Get(1, 1), m[5]);
    EXPECT_FLOAT_EQ(m.Get(2, 2), m[10]);
    EXPECT_FLOAT_EQ(m.Get(3, 3), m[15]);

    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            EXPECT_FLOAT_EQ(m.Get(i, j), m[i*4+j]);
        }
    }
}

TEST(Matrix4Test, Transpose) {
    Matrix4 m(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);
    Matrix4 mt = m.Transpose();
    // Check if mt is the transpose of m
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            EXPECT_FLOAT_EQ(m.Get(i, j), mt.Get(j,i));
        }
    }
}

TEST(Matrix4Test, InverseMultiplication) {
    Matrix4 m(-1, 2, 3, 4, 5, -6, 7, 8, 9, 10, -11, 12, 13, 14, 15, -16);
    Matrix4 inverse =  m.Inverse();
    Matrix4 identity;
    Matrix4 result = m * inverse;
    for (int i{}; i < 4; ++i) {
        for (int j{}; j < 4; ++j) {
            EXPECT_NEAR(result.Get(i,j), identity.Get(i,j), EPSILON);
        }
    }
}

TEST(Matrix4Test, MatrixMultiplication) {
    Matrix4 m1(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);
    Matrix4 m2(16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1);
    Matrix4 product = m1 * m2;
    const Matrix4 answer{ 386,444,502,560,274,316,358,400,162,188,214,240,50,60,70,80 };
    for (int i{}; i < 4; ++i) {
        for (int j{}; j < 4; ++j) {
            EXPECT_FLOAT_EQ(product.Get(i, j), answer.Get(i, j));
        }
    }
}


TEST(Matrix4Test, ScalarMultiplication) {
    Matrix4 m(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);
    float scalar = 2.f;
    Matrix4 result = m * scalar;
    const Matrix4 answer{2,4,6,8,10,12,14,16,18,20,22,24,26,28,30,32};

    for (int i{}; i < 4; ++i) {
        for (int j{}; j < 4; ++j) {
            EXPECT_FLOAT_EQ(result.Get(i, j), answer.Get(i, j));
        }
    }
}

TEST(Matrix4Test, Equality) {
    Matrix4 m1(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);
    Matrix4 m2(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);
    EXPECT_TRUE(m1 == m2);
}

TEST(Matrix4Test, Inequality) {
    Matrix4 m1(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);
    Matrix4 m2(16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1);
    EXPECT_TRUE(m1 != m2);
}

TEST(Matrix4Test, Extract3x3Matrix) {
    // Create and initialize a Matrix4 instance
    Matrix4 m(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);

    // Expected 3x3 matrix result
    Matrix3 expected = {
        1.0f, 2.0f, 3.0f,
        5.0f, 6.0f, 7.0f,
        9.0f, 10.0f, 11.0f
    };

    // Perform the extraction
    Matrix3 result = m.Extract3x3Matrix();

    // Check each element
    for (int col = 0; col < 3; ++col) {
        for (int row = 0; row < 3; ++row) {
            EXPECT_EQ(result.entries[col][row], expected.entries[col][row]);
        }
    }
}
TEST(Matrix4Test, ConvertToGLM) {
    // Create and initialize a Matrix4 instance
    Matrix4 m(1.f, 2.f, 3.f, 4.f, 5.f, 6.f, 7.f, 8.f, 9.f, 10.f, 11.f, 12.f, 13.f, 14.f, 15.f, 16.f);

    // Expected glm::mat4 result
    glm::mat4 expected {1.f,2.f,3.f,4.f,5.f,6.f,7.f,8.f,9.f,10.f,11.f,12.f,13.f,14.f,15.f,16.f};

    // Perform the conversion
    glm::mat4 result = m.ConvertToGLM();

    // Check each element
    for (int col = 0; col < 4; ++col) {
        for (int row = 0; row < 4; ++row) {
            EXPECT_NEAR(result[col][row], expected[col][row], EPSILON);
        }
    }
}
TEST(QuaternionTest, DefaultConstructor) {
    physics::Quaternion q;
    EXPECT_FLOAT_EQ(q.w, 1.0f);
    EXPECT_FLOAT_EQ(q.x, 0.0f);
    EXPECT_FLOAT_EQ(q.y, 0.0f);
    EXPECT_FLOAT_EQ(q.z, 0.0f);
}
TEST(QuaternionTest, ParameterizedConstructor) {
    physics::Quaternion q(1.0f, 2.0f, 3.0f, 4.0f);
    EXPECT_FLOAT_EQ(q.w, 1.0f);
    EXPECT_FLOAT_EQ(q.x, 2.0f);
    EXPECT_FLOAT_EQ(q.y, 3.0f);
    EXPECT_FLOAT_EQ(q.z, 4.0f);
}
TEST(QuaternionTest, Normalize) {
    physics::Quaternion q(0.0f, 3.0f, 4.0f, 0.0f);
    q.Normalize();
    float magnitude = std::sqrt(q.w * q.w + q.x * q.x + q.y * q.y + q.z * q.z);
    EXPECT_NEAR(magnitude, 1.0f, 1e-5);
}
TEST(QuaternionTest, Addition) {
    physics::Quaternion q1(1.0f, 2.0f, 3.0f, 4.0f);
    physics::Quaternion q2(2.0f, 3.0f, 4.0f, 5.0f);
    physics::Quaternion result = q1 + q2;
    EXPECT_FLOAT_EQ(result.w, 3.0f);
    EXPECT_FLOAT_EQ(result.x, 5.0f);
    EXPECT_FLOAT_EQ(result.y, 7.0f);
    EXPECT_FLOAT_EQ(result.z, 9.0f);
}
TEST(QuaternionTest, Multiplication) {
    physics::Quaternion q1(1.0f, 2.0f, 1.0f,3.0f);
    physics::Quaternion q2(1.0f, 0.5f, 0.5f, 0.75f);
    physics::Quaternion result = q1 * q2;
    
    EXPECT_FLOAT_EQ(result.w, -2.75);
    EXPECT_FLOAT_EQ(result.x, 1.75f);
    EXPECT_FLOAT_EQ(result.y, 1.5f);
    EXPECT_FLOAT_EQ(result.z, 4.25f);
}
TEST(QuaternionTest, ScalarMultiplication) {
    physics::Quaternion q(1.0f, 2.0f, 3.0f, 4.0f);
    float scalar = 2.0f;
    physics::Quaternion result = q * scalar;
    EXPECT_FLOAT_EQ(result.w, 2.0f);
    EXPECT_FLOAT_EQ(result.x, 4.0f);
    EXPECT_FLOAT_EQ(result.y, 6.0f);
    EXPECT_FLOAT_EQ(result.z, 8.0f);
}
TEST(QuaternionTest, RotateByVector) {
    physics::Quaternion q(1.0f, 0.0f, 0.0f, 0.0f); // Assuming q is an identity quaternion
    Math::Vector3 vec(1.0f, 2.0f, 3.0f);
    float deltaTime = 0.5f;

    // Normalize the vector
    Math::Vector3 axis = vec.Normalize();
    float length = vec.Length();
    float angle = length * deltaTime;

    // Calculate the rotation quaternion
    float cosHalfAngle = std::cos(angle * 0.5f);
    float sinHalfAngle = std::sin(angle * 0.5f);
    // (a rotation around a unit axis vector)
    physics::Quaternion rotation(cosHalfAngle,
        axis.x * sinHalfAngle,
        axis.y * sinHalfAngle,
        axis.z * sinHalfAngle);

    // Expected result is the original quaternion rotated by the rotation quaternion
    physics::Quaternion expected = q * rotation;// Quaternion multiplication

    // Perform the rotation
    physics::Quaternion result = q.RotateByVector(vec, deltaTime);

    EXPECT_NEAR(result.w, expected.w, EPSILON);
    EXPECT_NEAR(result.x, expected.x, EPSILON);
    EXPECT_NEAR(result.y, expected.y, EPSILON);
    EXPECT_NEAR(result.z, expected.z, EPSILON);
}

TEST(QuaternionTest, Conjugate) {
    physics::Quaternion q(1.0f, 2.0f, 3.0f, 4.0f);
    physics::Quaternion conjugate = q.Conjugate();

    EXPECT_FLOAT_EQ(conjugate.w, 1.0f);
    EXPECT_FLOAT_EQ(conjugate.x, -2.0f);
    EXPECT_FLOAT_EQ(conjugate.y, -3.0f);
    EXPECT_FLOAT_EQ(conjugate.z, -4.0f);
}

TEST(QuaternionTest, RotateVectorByQuaternion) {
    // Quaternion representing a 90-degree rotation around the z-axis
    float angle =  PI/ 2.f; // 90 degrees
    physics::Quaternion q(std::cos(angle / 2), 0, 0, std::sin(angle / 2));

    // A vector along the x-axis
    physics::Vector3 vec(1.0f, 0.0f, 0.0f);

    // RotateByVector the vector
    physics::Vector3 rotatedVec = q.RotateVectorByQuaternion(vec);

    // Expect the vector to now be along the y-axis
    EXPECT_NEAR(rotatedVec.x, 0.f, EPSILON);
    EXPECT_NEAR(rotatedVec.y, 1.f, EPSILON);
    EXPECT_NEAR(rotatedVec.z, 0.f, EPSILON);
}
