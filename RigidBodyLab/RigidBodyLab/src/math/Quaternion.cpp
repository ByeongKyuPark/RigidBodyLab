#include "math/Quaternion.h"
#include <glm/glm.hpp>//glm::radians
#include <cmath>

using namespace physics;

physics::Quaternion::Quaternion(float angleDegrees, Vector3 axis) {
    axis.Normalize();
    
    const float halfAngle = 0.5f * glm::radians(angleDegrees);
    const float sinHalfAngle = std::sin(halfAngle);

    w = cosf(halfAngle);
    x = sinHalfAngle * axis.x;
    y = sinHalfAngle * axis.y;
    z = sinHalfAngle * axis.z;
}

void Quaternion::Normalize()
{
    float magnitude = w*w + x*x + y*y + z*z;
    if (magnitude == 0.0f)
    {
        w = 1.0f;
        return;
    }
    magnitude = 1.0f / sqrtf(magnitude);

    w *= magnitude;
    x *= magnitude;
    y *= magnitude;
    z *= magnitude;
}

Quaternion Quaternion::RotateByScaledVector(const Vector3& vec, const float GRID_SCALE) const
{
    return *this * Quaternion(0.0f, vec.x * GRID_SCALE, vec.y * GRID_SCALE, vec.z * GRID_SCALE);
}

Quaternion Quaternion::operator+(const Quaternion& other) const
{
    Quaternion result;

    result.w = w + other.w;
    result.x = x + other.x;
    result.y = y + other.y;
    result.z = z + other.z;

    return result;
}

void Quaternion::operator+=(const Quaternion& other)
{
    w += other.w;
    x += other.x;
    y += other.y;
    z += other.z;
}

Quaternion Quaternion::operator*(const Quaternion& other) const
{
    Quaternion result;
    
    result.w = w*other.w - x*other.x - y*other.y - z*other.z;
    result.x = w*other.x + x*other.w + y*other.z - z*other.y;
    result.y = w*other.y - x*other.z + y*other.w + z*other.x;
    result.z = w*other.z + x*other.y - y*other.x + z*other.w;

    return result;
}

void Quaternion::operator*=(const Quaternion& other)
{
    Quaternion result;
    
    result.w = w*other.w - x*other.x - y*other.y - z*other.z;
    result.x = w*other.x + x*other.w + y*other.z - z*other.y;
    result.y = w*other.y - x*other.z + y*other.w + z*other.x;
    result.z = w*other.z + x*other.y - y*other.x + z*other.w;

    *this = result;
}

Quaternion Quaternion::operator*(const float value) const
{
    Quaternion result;

    result.w = w * value;
    result.x = x * value;
    result.y = y * value;
    result.z = z * value;

    return result;
}

void Quaternion::operator*=(const float value)
{
    w *= value;
    x *= value;
    y *= value;
    z *= value;
}
