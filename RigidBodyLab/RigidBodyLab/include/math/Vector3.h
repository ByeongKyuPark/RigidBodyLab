#pragma once

#include <initializer_list>
#include <ostream>
#include <stdexcept>

#include <glm/glm.hpp>

namespace Math {

    struct Vector3 {
        float x;
        float y;
        float z;

        Vector3(float _x=0.f, float _y=0.f, float _z=0.f);
        Vector3(std::initializer_list<float> list);
        Vector3(const glm::vec3& v) :x{ v.x }, y{ v.y }, z{ v.z } {}

        float Length() const;
        float LengthSquared() const;

        Vector3& Normalize();
        Vector3 Normalize() const;
        float Dot(const Vector3& rhs) const;
        Vector3 Cross(const Vector3& rhs) const;

        void Clear();

        Vector3 operator+(const Vector3& rhs) const;
        Vector3& operator+=(const Vector3& rhs);
        Vector3 operator-(const Vector3& rhs) const;
        Vector3& operator-=(const Vector3& rhs);
        Vector3 operator*(float scalar) const;
        Vector3& operator*=(float scalar);
        float operator[](unsigned int idx) const;
        float& operator[](unsigned int idx);
        Vector3 operator-() const;
        bool operator==(const Vector3& rhs) const;
        bool operator!=(const Vector3& rhs) const;
        operator glm::vec3() const;
        friend std::ostream& operator<<(std::ostream& os, const Vector3& v);
    };

} // namespace Math
