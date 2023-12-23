#pragma once
#include "Vector3.h"
#include "Matrix4.h"
#include "Quaternion.h"
namespace Core {
    using Math::Matrix4;
    using Math::Vector3;
    using Math::Quaternion;

    struct Transform {
        Matrix4 m_localToWorld;//RT only (no scale)
        Vector3 m_position;
        Quaternion m_orientation;
        Transform(const Vector3& _position = Vector3{}, const Quaternion& _ori = Quaternion{}) :m_position{ _position }, m_orientation{ _ori } {
            Update();
        }
        void Update();
        Vector3 GetAxis(int index)const;
    };
}