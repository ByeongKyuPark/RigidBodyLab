#pragma once

#include <math/Math.h>

namespace Rendering {

    enum class LightType {
        Directional = 0,
        Point,
        Spot
    };

    class Light {
    private:
        LightType m_Type;
        Vec3 m_Position;       // Used for point and spotlights
        Vec3 m_Direction;      // Used for directional and spotlights
        Vec4 m_Color;          // RGBA color of the light
        float m_Intensity;     // Intensity of the light

        //TODO:: Additional attributes like attenuation factors for point and spotlights

    public:
        Light(LightType type, const Vec3& m_position, const Vec3& direction, const Vec4& color, float intensity)
            : m_Type(type), m_Position(m_position), m_Direction(direction), m_Color(color), m_Intensity(intensity) {}

        // Getter and setter methods
        LightType GetType() const { return m_Type; }
        void SetType(LightType type) { m_Type = type; }

        const Vec3& GetPosition() const { return m_Position; }
        void SetPosition(const Vec3& m_position) { m_Position = m_position; }

        const Vec3& GetDirection() const { return m_Direction; }
        void SetDirection(const Vec3& direction) { m_Direction = direction; }

        const Vec4& GetColor() const { return m_Color; }
        void SetColor(const Vec4& color) { m_Color = color; }

        float GetIntensity() const { return m_Intensity; }
        void SetIntensity(float intensity) { m_Intensity = intensity; }
    };

}
