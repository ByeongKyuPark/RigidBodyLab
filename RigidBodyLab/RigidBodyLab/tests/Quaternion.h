#pragma once

#include "Vector3.h"
#include <cmath>

namespace physics {
    using Math::Vector3;

    struct Quaternion {
        float w, x, y, z;

        // Constructs a quaternion with default values representing no rotation (identity quaternion)
        Quaternion(float _w = 1.0f, float _x = 0.0f, float _y = 0.0f, float _z = 0.0f);

        // Constructs a quaternion from an angle (in degrees) and a rotation axis
        explicit Quaternion(float angleDegrees, const Vector3& axis);

        // Normalizes the quaternion to unit length
        void Normalize() noexcept;

        // Returns a new quaternion representing this quaternion rotated by a scaled vector
        Quaternion RotateByVector(const Vector3& vec, float gridScale) const;

        // Returns the conjugate of the quaternion
        Quaternion Conjugate() const;

        // Quaternion addition
        Quaternion operator+(const Quaternion& other) const;

        // Quaternion compound addition
        void operator+=(const Quaternion& other);

        // Quaternion multiplication
        Quaternion operator*(const Quaternion& other) const;

        // Quaternion compound multiplication
        void operator*=(const Quaternion& other);

        // Scalar multiplication
        Quaternion operator*(float scalar) const;

        // Scalar compound multiplication
        void operator*=(float scalar);

        Vector3 RotateVectorByQuaternion(const Vector3& vec);

    };
}
