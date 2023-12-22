#include <core/Projectile.h>
void Core::Projectile::Activate(const Vector3& position) {
    const static Vector3 offset{ 0.f, 7.5f,0.f };
    if (m_object) {
        RigidBody* rb = m_object->GetRigidBody();
        if (rb) {
            rb->SetPosition(position);
            rb->SetLinearVelocity(offset-position* INITIAL_SPEED); //toward the center of the world
        }
		m_isActive = true;
        m_object->GetCollider()->SetCollisionEnabled(true);
    }
}

void Core::Projectile::Deactivate() {
    m_isActive = false;
    m_object->GetCollider()->SetCollisionEnabled(false);
}
