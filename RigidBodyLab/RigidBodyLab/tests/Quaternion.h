#pragma once

#include "Vector3.h"

namespace physics
{
    using Math::Vector3;

    struct Quaternion
    {
        float w;
        float x;
        float y;
        float z;

        //q = w + xi + yj + zk, where i, j, and k are the imaginary units.
        //  w: The scalar component of the quaternion.
        //  x, y, and z : The vector components of the quaternion.
        // initialized to the identity quaternion, representing no rotation
        Quaternion(float _w=1.f, float _x=0.f, float _y=0.f, float _z=0.f)
            : w(_w), x(_x), y(_y), z(_z) {}
        Quaternion(float angleDegrees, Vector3 axis);
    
        void Normalize();
        Quaternion RotateByScaledVector(const Vector3& vec, const float GRID_SCALE) const;

        Quaternion operator+(const Quaternion& other) const;
        void operator+=(const Quaternion& other);

        Quaternion operator*(const Quaternion& other) const;
        void operator*=(const Quaternion& other);

        Quaternion operator*(const float value) const;
        void operator*=(const float value);
    };
} 