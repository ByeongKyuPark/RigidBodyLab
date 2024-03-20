#pragma once
#include <core/Object.h>

namespace Core {

    struct Projectile {
        static constexpr float INITIAL_SPEED = 30.f;
        static constexpr float PROJECTILE_MASS = 2.f;
        static constexpr float PROJECTILE_Y_OFFSET = 0.1f;//put projectiles slightly up so that it does not hide the camera

        bool m_isActive;
        bool m_hasKnockedOff;
        Object* m_object;//not an owner

        Projectile(Object* obj) : m_isActive(false), m_hasKnockedOff{false}, m_object(obj) {}

        void Activate(const Vector3& position);
        void Deactivate();
        Vec3 CalculateInitialVelocity();
    };

}
