#include "Transform.h"
void Core::Transform::Update()
{
    // First column
    m_localToWorld.columns[0].m128_f32[0] = 1.0f - 2.0f * (m_orientation.y * m_orientation.y + m_orientation.z * m_orientation.z);
    m_localToWorld.columns[0].m128_f32[1] = 2.0f * (m_orientation.x * m_orientation.y + m_orientation.w * m_orientation.z);
    m_localToWorld.columns[0].m128_f32[2] = 2.0f * (m_orientation.x * m_orientation.z - m_orientation.w * m_orientation.y);
    m_localToWorld.columns[0].m128_f32[3] = 0.0f; // Assuming homogeneous coordinate for direction vectors is 0

    // Second column
    m_localToWorld.columns[1].m128_f32[0] = 2.0f * (m_orientation.x * m_orientation.y - m_orientation.w * m_orientation.z);
    m_localToWorld.columns[1].m128_f32[1] = 1.0f - 2.0f * (m_orientation.x * m_orientation.x + m_orientation.z * m_orientation.z);
    m_localToWorld.columns[1].m128_f32[2] = 2.0f * (m_orientation.y * m_orientation.z + m_orientation.w * m_orientation.x);
    m_localToWorld.columns[1].m128_f32[3] = 0.0f;

    // Third column
    m_localToWorld.columns[2].m128_f32[0] = 2.0f * (m_orientation.x * m_orientation.z + m_orientation.w * m_orientation.y);
    m_localToWorld.columns[2].m128_f32[1] = 2.0f * (m_orientation.y * m_orientation.z - m_orientation.w * m_orientation.x);
    m_localToWorld.columns[2].m128_f32[2] = 1.0f - 2.0f * (m_orientation.x * m_orientation.x + m_orientation.y * m_orientation.y);
    m_localToWorld.columns[2].m128_f32[3] = 0.0f; 

    // Fourth column (position)
    m_localToWorld.columns[3].m128_f32[0] = m_position[0];
    m_localToWorld.columns[3].m128_f32[1] = m_position[1];
    m_localToWorld.columns[3].m128_f32[2] = m_position[2];
    m_localToWorld.columns[3].m128_f32[3] = 1.0f; // Homogeneous coordinate for position is 1
}

Math::Vector3 Core::Transform::GetAxis(int index) const
{
    if (index < 0 || index > 3) {
        throw std::runtime_error("RigidBody::getAxis(): index, out of bounds\n");
    }

    Vector3 result(
        m_localToWorld.columns[index].m128_f32[0],
        m_localToWorld.columns[index].m128_f32[1],
        m_localToWorld.columns[index].m128_f32[2]
    );
    result.Normalize();

    return result;
}
