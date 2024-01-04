#include <rendering/OrbitalLight.h>
using namespace Rendering;

glm::vec3 Rendering::OrbitalLight::RotateAroundAxis(const glm::vec3& point, float angleDegrees) const {
    glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(angleDegrees), m_rotationAxis);
    glm::vec4 rotatedPoint = rotationMatrix * glm::vec4(point, 1.0f);
    return glm::vec3(rotatedPoint);
}

void Rendering::OrbitalLight::UpdatePosition(float dt) {
    m_accumulatedTime += dt * m_orbitalSpeed;

    Vec3 newPos = Vec3(
        m_lightOrbitOffset.x + cos(m_accumulatedTime + m_lightOrbitOffset.y) * m_orbitalRad,
        m_lightPosWF.y + sin(m_accumulatedTime + m_lightOrbitOffset.z) * m_orbitalRad,
        m_lightOrbitOffset.z + sin(m_accumulatedTime + m_lightOrbitOffset.x) * m_orbitalRad
    );

    // Apply rotation to the orbit path
    newPos = RotateAroundAxis(newPos , m_rotationAngle);

    // Update world frame position
    m_lightPosWF = newPos;
}
