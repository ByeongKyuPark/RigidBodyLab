#pragma once
#include <core/Object.h>

namespace Core {

    struct Projectile {
        constexpr static float INITIAL_SPEED = 1.5f;

        bool m_isActive;
        Object* m_object;//not an owner

        Projectile(Object* obj) : m_isActive(false), m_object(obj) {}

        void Activate(const Vector3& position);
        void Deactivate();
    };

}
