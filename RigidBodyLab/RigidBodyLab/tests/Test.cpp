#include"gtest/gtest.h"
#include "Vector3.h"
#include "Quaternion.h"
#include "Matrix3.h"
#include "Matrix4.h"

const float EPSILON = 1e-5f;
const float LOOSE_EPSILON = 1e-3f;

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
