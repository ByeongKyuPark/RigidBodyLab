#include <core/Projectile.h>
#include <rendering/Camera.h>

void Core::Projectile::Activate(const Vector3& position) {
    Vec3 camPos = mainCam.GetPos();
    Vec3 lookAtVec = mainCam.GetLookAtVec();
    Vec3 upVec = mainCam.GetUpVec();

    Vec3 forwardDirection = Normalize(lookAtVec - camPos);

    // launch at a 45-degree angle upward
    Vec3 launchDirection = Normalize(forwardDirection + upVec);

    Vec3 initialVelocity = launchDirection * INITIAL_SPEED;

    if (m_object) {
        RigidBody* rb = m_object->GetRigidBody();
        if (rb) {
            // adj pos slightly above the camera (so as not to hide the screen)
            rb->SetPosition(camPos + Vec3(0.0f, 0.2f, 0.0f));
            rb->SetLinearVelocity(initialVelocity);
        }
        m_isActive = true;
        m_object->GetCollider()->SetCollisionEnabled(true);
        m_object->SetVisibility(true);
    }
}


void Core::Projectile::Deactivate() {
    m_isActive = false;
    m_object->GetCollider()->SetCollisionEnabled(false);
    m_object->SetVisibility(false);
}
