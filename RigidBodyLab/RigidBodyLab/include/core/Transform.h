#pragma once
#include <math/Vector3.h>
#include <math/Matrix4.h>
#include <math/Quaternion.h>
namespace Core {
    using Math::Matrix4;
    using Math::Vector3;
    using Math::Quaternion;

    struct Transform {
        Matrix4 localToWorld;//RT only (no scale)
        Vector3 position;
        Quaternion orientation;
        Transform(const Vector3& _position = Vector3{}, const Quaternion& _ori = Quaternion{}) :position{ _position }, orientation{ _ori } {
            Update();
        }
        void Update();
        Vector3 GetAxis(int index)const;
    };
}