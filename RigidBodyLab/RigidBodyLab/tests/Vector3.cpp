// Vector3.cpp
#include "math/Vector3.h"
#include <cmath>
#include <iostream>

namespace Math {

    Vector3::Vector3(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}

    Vector3::Vector3(std::initializer_list<float> list) {
        if (list.size() < 3) {
            throw std::invalid_argument("Initializer list should have at least 3 elements.");
        }
        auto iter = list.begin();
        x = *iter++;
        y = *iter++;
        z = *iter;
    }

    float Vector3::Length() const {
        return std::sqrt(x * x + y * y + z * z);
    }

    float Vector3::LengthSquared() const {
        return x * x + y * y + z * z;
    }

    Vector3& Vector3::Normalize() {
        float length = Length();
        if (length != 0.f) {
            float invLength = 1.f / length;
            x *= invLength;
            y *= invLength;
            z *= invLength;
        }
        return *this;
    }

    Vector3 Vector3::Normalize() const {
        float length = std::sqrt(x * x + y * y + z * z);

        // Check for division by zero
        if (length > std::numeric_limits<float>::epsilon()) {
            return Vector3(x / length, y / length, z / length);
        }
        else {
            // Return zero vector if original length is zero
            return Vector3(0.0f, 0.0f, 0.0f);
        }
    }

    float Vector3::Dot(const Vector3& rhs) const {
        return x * rhs.x + y * rhs.y + z * rhs.z;
    }

    Vector3 Vector3::Cross(const Vector3& rhs) const {
        return Vector3(
            y * rhs.z - z * rhs.y,
            z * rhs.x - x * rhs.z,
            x * rhs.y - y * rhs.x
        );
    }

    void Vector3::Clear() {
        x = y = z = 0.f;
    }

    Vector3 Vector3::operator+(const Vector3& rhs) const {
        return Vector3(x + rhs.x, y + rhs.y, z + rhs.z);
    }

    Vector3& Vector3::operator+=(const Vector3& rhs) {
        x += rhs.x;
        y += rhs.y;
        z += rhs.z;
        return *this;
    }

    Vector3 Vector3::operator-(const Vector3& rhs) const {
        return Vector3(x - rhs.x, y - rhs.y, z - rhs.z);
    }

    Vector3& Vector3::operator-=(const Vector3& rhs) {
        x -= rhs.x;
        y -= rhs.y;
        z -= rhs.z;
        return *this;
    }

    Vector3 Vector3::operator*(float scalar) const {
        return Vector3(x * scalar, y * scalar, z * scalar);
    }

    Vector3& Vector3::operator*=(float scalar) {
        x *= scalar;
        y *= scalar;
        z *= scalar;
        return *this;
    }

    float Vector3::operator[](unsigned int idx) const {
        switch (idx) {
        case 0: return x;
        case 1: return y;
        case 2: return z;
        default: throw std::out_of_range("Index out of range");
        }
    }

    float& Vector3::operator[](unsigned int idx) {
        switch (idx) {
        case 0: return x;
        case 1: return y;
        case 2: return z;
        default: throw std::out_of_range("Index out of range");
        }
    }

    Vector3 Vector3::operator-() const {
        return { -x, -y, -z };
    }

    bool Vector3::operator==(const Vector3& rhs) const {
        return x == rhs.x && y == rhs.y && z == rhs.z;
    }

    bool Vector3::operator!=(const Vector3& rhs) const {
        return !(*this == rhs);
    }

    std::ostream& operator<<(std::ostream& os, const Vector3& v) {
        os << "(" << v.x << ", " << v.y << ", " << v.z << ")";
        return os;
    }
} // namespace Math
