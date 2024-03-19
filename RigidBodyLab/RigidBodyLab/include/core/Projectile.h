#pragma once
#include <core/Object.h>

namespace Core {

    struct Projectile {
        constexpr static float INITIAL_SPEED = 2.f;

        bool m_isActive;
        bool m_hasKnockedOff;
        Object* m_object;//not an owner

        Projectile(Object* obj) : m_isActive(false), m_hasKnockedOff{false}, m_object(obj) {}

        void Activate(const Vector3& position);
        void Deactivate();
    };

}
