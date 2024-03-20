#include <core/Projectile.h>
#include <rendering/Camera.h>

void Core::Projectile::Activate(const Vector3& position) {
    if (m_object) {
        RigidBody* rb = m_object->GetRigidBody();
        if (rb) {
            // adj pos slightly above the camera (so as not to hide the screen)
            rb->SetPosition(mainCam.GetPos() + Vec3(0.0f, PROJECTILE_Y_OFFSET, 0.0f));
            rb->SetLinearVelocity(CalculateInitialVelocity());
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

Vec3 Core::Projectile::CalculateInitialVelocity() {
    Vec3 camPos = mainCam.GetPos();
    Vec3 lookAtVec = mainCam.GetLookAtVec();
    Vec3 upVec = mainCam.GetUpVec();

    // calculate the normalized forward direction from the camera to its look-at point
    Vec3 forwardDirection = Normalize(lookAtVec - camPos);

    // scale down the up vector's influence for a more horizontal launch
    const float UP_VECTOR_SCL_DOWN_FACTOR = 0.25f;
    Vec3 launchDirection = Normalize(forwardDirection + upVec * UP_VECTOR_SCL_DOWN_FACTOR);

    Vec3 initialVelocity = launchDirection * INITIAL_SPEED;

    return initialVelocity;
}
