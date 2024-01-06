#include <rendering/OrbitalLight.h>
using namespace Rendering;

void Rendering::OrbitalLight::Update(float dt) {
    m_accumulatedTime += dt * m_orbitalSpeed;

    // calc new position in a circular orbit around the Y-axis
    float x = cos(m_accumulatedTime) * m_orbitalRad;
    float y = m_lightOrbitOffset.y; // const
    float z = sin(m_accumulatedTime) * m_orbitalRad;

    m_lightPosWF = Vec3(x, y, z);

    m_lightView = glm::lookAt(m_lightPosWF, glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));

    m_lightSpaceMat = m_lightProjection * m_lightView;
 
    // update color
    constexpr float COLOR_CHANGE_SPEED = 2.0f; 
    constexpr float COLOR_INTENSITY_MAX = 0.85f;
    m_intensity.r = (sin(m_accumulatedTime * COLOR_CHANGE_SPEED) + 1.0f) / 2.0f * COLOR_INTENSITY_MAX;
    m_intensity.g = (cos(m_accumulatedTime * COLOR_CHANGE_SPEED) + 1.0f) / 2.0f * COLOR_INTENSITY_MAX;
    m_intensity.b = (sin(m_accumulatedTime * COLOR_CHANGE_SPEED + HALF_PI) + 1.0f) / 2.0f * COLOR_INTENSITY_MAX;
}
