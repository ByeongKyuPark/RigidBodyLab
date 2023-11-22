#include "math/Quaternion.h"
#include <glm/glm.hpp> // For glm::radians

using namespace physics;

Quaternion::Quaternion(float _w, float _x, float _y, float _z) : w(_w), x(_x), y(_y), z(_z) {}

Quaternion::Quaternion(float angleDegrees, const Vector3& axis) {
    Vector3 normAxis = axis.Normalize();

    float halfAngle = 0.5f * glm::radians(angleDegrees);
    float sinHalfAngle = std::sin(halfAngle);

    w = std::cos(halfAngle);
    x = sinHalfAngle * normAxis.x;
    y = sinHalfAngle * normAxis.y;
    z = sinHalfAngle * normAxis.z;
}

void Quaternion::Normalize() noexcept {
    float mag = w * w + x * x + y * y + z * z;
    if (mag > std::numeric_limits<float>::epsilon()) {
        mag = std::sqrt(mag);
        w /= mag;
        x /= mag;
        y /= mag;
        z /= mag;
    }
    else {
        w = 1.f;
    }
}

Quaternion Quaternion::RotateByVector(const Vector3& vec, float deltaTime) const {
    // Compute the angle of rotation (magnitude of angular velocity * time)
    float angle = vec.Length() * deltaTime;

    // If the angle is very small, the rotation can be approximated as no rotation
    if (angle < std::numeric_limits<float>::epsilon()) {
        return *this;
    }

    // Normalize the axis of rotation
    Vector3 axis = vec.Normalize();

    // Create a quaternion representing this rotation
    Quaternion rotation(std::cos(angle * 0.5f),
        axis.x * std::sin(angle * 0.5f),
        axis.y * std::sin(angle * 0.5f),
        axis.z * std::sin(angle * 0.5f));

    // Combine the rotations
    return *this * rotation;
}

Quaternion Quaternion::Conjugate() const {
    return Quaternion(w, -x, -y, -z);
}

Quaternion Quaternion::operator+(const Quaternion& other) const {
    return { w + other.w, x + other.x, y + other.y, z + other.z };
}

void Quaternion::operator+=(const Quaternion& other) {
    w += other.w;
    x += other.x;
    y += other.y;
    z += other.z;
}

Quaternion Quaternion::operator*(const Quaternion& other) const {
    return {
        w * other.w - x * other.x - y * other.y - z * other.z,
        w * other.x + x * other.w + y * other.z - z * other.y,
        w * other.y - x * other.z + y * other.w + z * other.x,
        w * other.z + x * other.y - y * other.x + z * other.w
    };
}

void Quaternion::operator*=(const Quaternion& other) {
    *this = *this * other;
}

Quaternion Quaternion::operator*(float scalar) const {
    return { w * scalar, x * scalar, y * scalar, z * scalar };
}

void Quaternion::operator*=(float scalar) {
    w *= scalar;
    x *= scalar;
    y *= scalar;
    z *= scalar;
}

Vector3 physics::Quaternion::RotateVectorByQuaternion(const Vector3& vec) {
    // Convert vector to a quaternion with zero scalar part
    Quaternion vecQuat(0, vec.x, vec.y, vec.z);

    // RotateByVector the vector quaternion by the rotation quaternion and its conjugate
    Quaternion rotatedVecQuat = *this * vecQuat * Conjugate();

    // Return the vector part of the rotated quaternion
    return Vector3(rotatedVecQuat.x, rotatedVecQuat.y, rotatedVecQuat.z);
}
