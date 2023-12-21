#include "Transform.h"

using Math::Matrix4;
using Math::Vector3;
using Math::Quaternion;

void Core::Transform::Update()
{
    // First column
    localToWorld.columns[0].m128_f32[0] = 1.0f - 2.0f * (orientation.y * orientation.y + orientation.z * orientation.z);
    localToWorld.columns[0].m128_f32[1] = 2.0f * (orientation.x * orientation.y + orientation.w * orientation.z);
    localToWorld.columns[0].m128_f32[2] = 2.0f * (orientation.x * orientation.z - orientation.w * orientation.y);
    localToWorld.columns[0].m128_f32[3] = 0.0f; // Assuming homogeneous coordinate for direction vectors is 0

    // Second column
    localToWorld.columns[1].m128_f32[0] = 2.0f * (orientation.x * orientation.y - orientation.w * orientation.z);
    localToWorld.columns[1].m128_f32[1] = 1.0f - 2.0f * (orientation.x * orientation.x + orientation.z * orientation.z);
    localToWorld.columns[1].m128_f32[2] = 2.0f * (orientation.y * orientation.z + orientation.w * orientation.x);
    localToWorld.columns[1].m128_f32[3] = 0.0f; // Assuming homogeneous coordinate for direction vectors is 0

    // Third column
    localToWorld.columns[2].m128_f32[0] = 2.0f * (orientation.x * orientation.z + orientation.w * orientation.y);
    localToWorld.columns[2].m128_f32[1] = 2.0f * (orientation.y * orientation.z - orientation.w * orientation.x);
    localToWorld.columns[2].m128_f32[2] = 1.0f - 2.0f * (orientation.x * orientation.x + orientation.y * orientation.y);
    localToWorld.columns[2].m128_f32[3] = 0.0f; // Assuming homogeneous coordinate for direction vectors is 0

    // Fourth column (position)
    localToWorld.columns[3].m128_f32[0] = position[0];
    localToWorld.columns[3].m128_f32[1] = position[1];
    localToWorld.columns[3].m128_f32[2] = position[2];
    localToWorld.columns[3].m128_f32[3] = 1.0f; // Homogeneous coordinate for position is 1
}

Vector3 Core::Transform::GetAxis(int index) const {
    if (index < 0 || index > 2) {
        throw std::runtime_error("Transform::GetAxis(): index, out of bounds\n");
    }
    Vector3 result(
        localToWorld[index * 4],
        localToWorld[index * 4 + 1],
        localToWorld[index * 4 + 2]
    );
    result.Normalize();

    return result;

}
