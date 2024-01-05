#pragma once

#include <math/Math.h>
namespace Rendering {
    struct OrbitalLight {
        Mat4 m_lightSpaceMat;
        Mat4 m_lightProjection;
        Mat4 m_lightView;
        Vec3 m_lightOrbitOffset;
        Vec3 m_lightPosWF;  // World Frame position
        Vec3 m_lightPosVF;  // View Frame position
        Vec3 m_rotationAxis;
        Vec4 m_intensity;
        float m_orbitalRad;
        float m_orbitalSpeed;
        float m_accumulatedTime;
        float m_rotationAngle;

        OrbitalLight() :m_lightSpaceMat{ 1.f }, m_lightProjection{ 1.f }, m_lightView{1.f}, m_lightOrbitOffset {}, m_lightPosVF{}, m_lightPosWF{}, m_orbitalRad(1.0f), m_rotationAxis{ 0.0f, 1.0f, 0.0f },
            m_orbitalSpeed(1.0f), m_accumulatedTime(0.0f), m_intensity{0.f,0.f,0.f,1.f}, m_rotationAngle(0.0f)  
        {
            m_lightProjection = glm::perspective(glm::radians(45.0f), 1.0f, 0.f, 3.0f);
        }

        glm::vec3 RotateAroundAxis(const glm::vec3& point, float angleDegrees) const;

        void UpdatePosition(float dt);
    };

}