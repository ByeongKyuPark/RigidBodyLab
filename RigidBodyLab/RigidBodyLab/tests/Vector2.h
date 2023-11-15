#pragma once

#include <iostream>
#include <cmath>

class Vector2 {
public:
    float x, y;

    // Constructors
    Vector2(float x = 0.0f, float y = 0.0f);

    // Copy constructor and copy assignment operator
    Vector2(const Vector2& other) = default;
    Vector2& operator=(const Vector2& other) = default;

    // Move constructor and move assignment operator
    Vector2(Vector2&& other) noexcept = default;
    Vector2& operator=(Vector2&& other) noexcept = default;

    // Basic arithmetic operations
    Vector2 operator+(const Vector2& rhs) const;
    Vector2& operator+=(const Vector2& rhs);
    Vector2 operator-(const Vector2& rhs) const;
    Vector2& operator-=(const Vector2& rhs);
    Vector2 operator*(float scalar) const;
    Vector2& operator*=(float scalar);

    // Dot product
    float Dot(const Vector2& rhs) const;

    // Vector magnitude and normalization
    float Length() const;
    float LengthSquared() const;
    Vector2& Normalize();

    // Utility functions
    friend std::ostream& operator<<(std::ostream& os, const Vector2& v);
};