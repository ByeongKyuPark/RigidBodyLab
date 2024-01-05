#include <rendering/OrbitalLight.h>
using namespace Rendering;

Vec3 Rendering::OrbitalLight::RotateAroundAxis(const Vec3& point, float angleDegrees) const {
    Mat4 rotationMatrix = glm::rotate(Mat4(1.0f), glm::radians(angleDegrees), m_rotationAxis);
    Vec4 rotatedPoint = rotationMatrix * Vec4(point, 1.0f);
    return Vec3(rotatedPoint);
}

void Rendering::OrbitalLight::UpdatePosition(float dt) {
    m_accumulatedTime += dt * m_orbitalSpeed;

    Vec3 newPos = Vec3(
        m_lightOrbitOffset.x + cos(m_accumulatedTime + m_lightOrbitOffset.y) * m_orbitalRad,
        m_lightPosWF.y,
        m_lightOrbitOffset.z + sin(m_accumulatedTime + m_lightOrbitOffset.x) * m_orbitalRad
    );

    newPos = RotateAroundAxis(newPos , m_rotationAngle);
    m_lightPosWF = newPos;

    m_lightView = glm::lookAt(m_lightPosWF, glm::vec3(0.0), glm::vec3(0.0, 1.0, 0.0)); 
    m_lightSpaceMat = m_lightProjection * m_lightView;
}
