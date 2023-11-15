#include "Vector2.h"

// Constructor
Vector2::Vector2(float x, float y) : x(x), y(y) {}

// Basic arithmetic operations
Vector2 Vector2::operator+(const Vector2& rhs) const {
    return Vector2(x + rhs.x, y + rhs.y);
}

Vector2& Vector2::operator+=(const Vector2& rhs) {
    x += rhs.x;
    y += rhs.y;
    return *this;
}

Vector2 Vector2::operator-(const Vector2& rhs) const {
    return Vector2(x - rhs.x, y - rhs.y);
}

Vector2& Vector2::operator-=(const Vector2& rhs) {
    x -= rhs.x;
    y -= rhs.y;
    return *this;
}

Vector2 Vector2::operator*(float scalar) const {
    return Vector2(x * scalar, y * scalar);
}

Vector2& Vector2::operator*=(float scalar) {
    x *= scalar;
    y *= scalar;
    return *this;
}

// Dot product
float Vector2::Dot(const Vector2& rhs) const {
    return x * rhs.x + y * rhs.y;
}

// Vector magnitude and normalization
float Vector2::Length() const {
    return std::sqrt(x * x + y * y);
}

float Vector2::LengthSquared() const {
    return x * x + y * y;
}

Vector2& Vector2::Normalize() {
    float len = Length();
    if (len != 0) {
        x /= len;
        y /= len;
    }
    return *this;
}

// Utility functions
std::ostream& operator<<(std::ostream& os, const Vector2& v) {
    os << "Vector2(" << v.x << ", " << v.y << ")";
    return os;
}
