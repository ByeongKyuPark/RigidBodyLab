#pragma once

#include <iostream>
#include <iomanip>
#include <cmath>

#define GLM_FORCE_RADIANS

#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtc/matrix_transform.hpp"


using Vec2 = glm::vec2;
using Vec3 = glm::vec3;
using Vec4 = glm::vec4;
using Mat3 = glm::mat3;
using Mat4 = glm::mat4;


/*  Useful constants */
constexpr float  PI             = 3.14159f;
constexpr float  TWO_PI         = 2.0f * PI;
constexpr float  HALF_PI        = 0.5f * PI;
constexpr float  QUARTER_PI     = 0.25f * PI;
constexpr float  EIGHTH_PI      = 0.125f * PI;
constexpr float  FIFTHTEENTH_PI = 0.0667f * PI;
constexpr float  SIXTEENTH_PI   = 0.0625f * PI;

constexpr float  DEG_TO_RAD = PI / 180.0f;
constexpr float  RAD_TO_DEG = 180.0f / PI;

constexpr float  EPSILON = 0.00001f;

constexpr int X = 0;
constexpr int Y = 1;
constexpr int Z = 2;

const Vec3 BASIS[3]{    Vec3(1.0f, 0.0f, 0.0f),
                        Vec3(0.0f, 1.0f, 0.0f),
                        Vec3(0.0f, 0.0f, 1.0f)
                   };


/******************************************************************************/
/*  Utility functions                                                         */
/******************************************************************************/
float RoundDecimal(float input);
Vec3 RoundDecimal(const Vec3 &input);
Vec4 RoundDecimal(const Vec4 &input);
bool DegenerateTri(const Vec3 &v0, const Vec3 &v1, const Vec3 &v2);


/******************************************************************************/
/*  Wrappers for GLM functions                                                */
/******************************************************************************/

//Mat4 Scale(float sx, float sy, float sz);
Mat4 Scale(const Vec3 &s);

Mat4 Rotate(float angle, const Vec3 &axis);

Mat4 Translate(const Vec3 &t);

Mat4 Frustum(float leftPlane, float rightPlane, 
             float bottomPlane, float topPlane, 
             float nearPlane, float farPlane);
Mat4 Perspective(float fovy, float aspect, float near, float far);

Mat4 LookAt(const Vec3 &eye, const Vec3 &center, const Vec3 &up);

Mat4 Inverse(const Mat4 &m);
Mat4 Transpose(const Mat4 &m);

Vec3 Normalize(const Vec3 &v);
Vec3 Cross(const Vec3 &v1, const Vec3 &v2);
Vec3 Max(const Vec3& a, const Vec3& b);
Vec3 Min(const Vec3& a, const Vec3& b);

float Dot(const Vec3 &v1, const Vec3 &v2);
float Distance(const Vec3 &v1, const Vec3 &v2);

template <typename T>
float *ValuePtr(T value) {  return glm::value_ptr(value);   }