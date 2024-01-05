#include <rendering/OrbitalLight.h>
using namespace Rendering;

void Rendering::OrbitalLight::UpdatePosition(float dt) {
    m_accumulatedTime += dt * m_orbitalSpeed;

    // calc new position in a circular orbit around the Y-axis
    float x = cos(m_accumulatedTime) * m_orbitalRad;
    float y = m_lightOrbitOffset.y; // const
    float z = sin(m_accumulatedTime) * m_orbitalRad;

    m_lightPosWF = Vec3(x, y, z);

    m_lightView = glm::lookAt(m_lightPosWF, glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));

    m_lightSpaceMat = m_lightProjection * m_lightView;
}
