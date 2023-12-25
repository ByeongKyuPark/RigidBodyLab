#include <physics/CollisionManager.h>
#include <utilities/ThreadPool.h>

Math::Vector3 Physics::CollisionManager::GetBoxContactVertexLocal(const Vector3& axis1, const Vector3& axis2, const Vector3& axis3, Vector3 collisionNormal, std::function<bool(float, float)> cmp) const {
    Vector3 contactPoint{ 0.5f,0.5f,0.5f };

    if (cmp(axis1.Dot(collisionNormal), 0)) {
        contactPoint.x *= -1.0f;
    }
    if (cmp(axis2.Dot(collisionNormal), 0)) {
        contactPoint.y *= -1.0f;
    }
    if (cmp(axis3.Dot(collisionNormal), 0)) {
        contactPoint.z *= -1.0f;
    }
    return contactPoint;
}

float Physics::CollisionManager::CalcPenetration(Vec3& extents1, Vec3& extents2, Vector3& pos1, Vector3& pos2, const std::vector<Vector3>& axes, int axisIdx) {

    Vector3 centerToCenter = pos2 - pos1;
    float projectedCenterToCenter = abs(centerToCenter.Dot(axes[axisIdx]));


    float projectedSum =
        abs((axes[0] * extents1.x).Dot(axes[axisIdx])) +
        abs((axes[1] * extents1.y).Dot(axes[axisIdx])) +
        abs((axes[2] * extents1.z).Dot(axes[axisIdx])) +

        abs((axes[3] * extents2.x).Dot(axes[axisIdx])) +
        abs((axes[4] * extents2.y).Dot(axes[axisIdx])) +
        abs((axes[5] * extents2.z).Dot(axes[axisIdx]));

    return projectedSum - projectedCenterToCenter;
}

// Function to handle Box-Box collision
void Physics::CollisionManager::CalcContactPointsBoxBox(const BoxCollider& box1, const BoxCollider& box2, const Object* obj1, const Object* obj2, CollisionData& newContact, int minPenetrationAxisIdx, const std::vector<Vector3>& axes) const {
    //  	1. for cases 0 to 5, vertices are found to define contact points
    if (minPenetrationAxisIdx >= 0 && minPenetrationAxisIdx < 3)
    {
        //Vec3 scl = std::get<Vec3>(box2.GetScale());
        Vector3 contactPoint = GetBoxContactVertexLocal(axes[3], axes[4], axes[5], newContact.collisionNormal, Less);
        contactPoint = obj2->GetUnitModelMatrix() * contactPoint;
        newContact.contactPoint = { { true, contactPoint + newContact.collisionNormal * newContact.penetrationDepth },
            { true, contactPoint } };
    }
    else if (minPenetrationAxisIdx >= 3 && minPenetrationAxisIdx < 6) {
        //Vec3 scl = std::get<Vec3>(box1.GetScale());
        Vector3 contactPoint = GetBoxContactVertexLocal(axes[0], axes[1], axes[2], newContact.collisionNormal, Greater);
        contactPoint = obj1->GetUnitModelMatrix() * contactPoint;

        newContact.contactPoint = { { true, contactPoint },
            { true, contactPoint - newContact.collisionNormal * newContact.penetrationDepth } };
    }
    //    2. for cases 6 to 15, points on the edges of the bounding boxes are used.
    else
    {
        // Determine the local contact vertex on box1 based on the collision's hit normal.
        // hit normal : box1 < -- - box2
        // For example:
        // If the dot product of box1's X-axis and the hit normal is positive,
        // it indicates that the collision is happening on the negative side of the X-axis.
        // In such a case, the x-coordinate of the contact vertex (vertexOne) should be negative 
        // to represent the correct side of the box where the collision occurred.
        Vector3 vertexOne = GetBoxContactVertexLocal(axes[0], axes[1], axes[2], newContact.collisionNormal, Greater);
        Vector3 vertexTwo = GetBoxContactVertexLocal(axes[3], axes[4], axes[5], newContact.collisionNormal, Less);

        int testAxis1{ -1 }, testAxis2{ -1 };

        switch (minPenetrationAxisIdx)
        {
        case 6: // x,x
            testAxis1 = 0;
            testAxis2 = 0;
            break;

        case 7: // x,y
            testAxis1 = 0;
            testAxis2 = 1;
            break;

        case 8: // x,z
            testAxis1 = 0;
            testAxis2 = 2;
            break;

        case 9: // y,x
            testAxis1 = 1;
            testAxis2 = 0;
            break;

        case 10: // y,y
            testAxis1 = 1;
            testAxis2 = 1;
            break;

        case 11: // y,z
            testAxis1 = 1;
            testAxis2 = 2;
            break;

        case 12: // z,x
            testAxis1 = 2;
            testAxis2 = 0;
            break;

        case 13: // z,y
            testAxis1 = 2;
            testAxis2 = 1;
            break;

        case 14: // z,z
            testAxis1 = 2;
            testAxis2 = 2;
            break;

        default:
            break;
        }
        //orientation of the colliding edges (not the moving away)
        Vector3 edge1, edge2;//world space
        //ex. case 6(x,x): vertexTwo.x <0, then local x-axis should be flipped
        edge1 = (vertexOne[testAxis1] < 0) ? axes[testAxis1] : axes[testAxis1] * -1.f;
        edge2 = (vertexTwo[testAxis2] < 0) ? axes[testAxis2] : axes[testAxis2] * -1.f;

        //local -> world
        vertexOne = obj1->GetUnitModelMatrix() * vertexOne;
        vertexTwo = obj2->GetUnitModelMatrix() * vertexTwo;

        //1. calculate the dot product between edge1 and edge2:
        float k = edge1.Dot(edge2);//cosine

        //2.  point on the edge of box1 closest to the initial contact point on box2
        //    The calculation involves projecting the vector from contactPoint1 to contactPoint2 onto the direction of edge1 - edge2 * k.
        Vector3 closestPointOne = { vertexOne + edge1 * ((vertexTwo - vertexOne).Dot(edge1 - edge2 * k) / (1 - k * k)) };

        //3. point on the edge of box2 closest to 
        //projecting the vector from closestPointOne to vertexTwo onto direction2.
        Vector3 closestPointTwo{ vertexTwo + edge2 * ((closestPointOne - vertexTwo).Dot(edge2)) };
        newContact.contactPoint = { { true, closestPointOne },
            { true, closestPointTwo } };
    }
}

void Physics::CollisionManager::FindCollisionFeaturesSphereBox(const SphereCollider* sphere, const BoxCollider* box, Object* sphereObj, Object* boxObj) {
    Vector3 spherePos = sphereObj->GetPosition();
    Vector3 boxPos = boxObj->GetPosition();

    constexpr int NUM_AXES = 3;
    Vec3 boxExtents = std::get<Vec3>(box->GetScale());//std::variant<Vec3, float>

    Vector3 closestPoint = boxPos;
    Vector3 centerToCenter = spherePos - boxPos;
    float radius = std::get<float>(sphere->GetScale());//std::variant<Vec3, float>

    // For each axis (X, Y, Z)
    for (int i = 0; i < NUM_AXES; ++i) {
        Vector3 axis = boxObj->GetAxis(i);
        float projectionLength = centerToCenter.Dot(axis);
        projectionLength = std::clamp(projectionLength, -boxExtents[i], boxExtents[i]);
        closestPoint += axis * projectionLength;
    }

    // Calculate squared distance to sphere center
    float distanceSquared = (closestPoint - spherePos).LengthSquared();

    if (distanceSquared > radius * radius) {
        return; // No collision
    }

    // If a collision is detected, populate and return CollisionData
    CollisionData collisionData;
    collisionData.objects[0] = sphereObj;
    collisionData.objects[1] = boxObj;
    collisionData.collisionNormal = spherePos - closestPoint;
    collisionData.collisionNormal.Normalize();
    collisionData.contactPoint = {
        { true, Vector3(spherePos - collisionData.collisionNormal * radius) },
        { true, Vector3(closestPoint) }
    };

    collisionData.penetrationDepth = radius - std::sqrt(distanceSquared);
    collisionData.restitution = m_objectRestitution;
    collisionData.friction = m_friction;
    AddCollision(collisionData);
}

void Physics::CollisionManager::FindCollisionFeaturesBoxBox(const BoxCollider* box1, const BoxCollider* box2, Object* obj1, Object* obj2) {

    static constexpr int NUM_AXES = 15;

    Vec3 extents1 = std::get<Vec3>(box1->GetScale());
    Vec3 extents2 = std::get<Vec3>(box2->GetScale());

    float radius1 = std::max({ extents1.x, extents1.y, extents1.z });
    float radius2 = std::max({ extents2.x, extents2.y, extents2.z });
    radius1 *= sqrt(2); //actual bounding sphere is bigger than the cube
    radius2 *= sqrt(2);//actual bounding sphere is bigger than the cube

    Vector3 position1 = obj1->GetPosition();
    Vector3 position2 = obj2->GetPosition();

    Vector3 distanceVec = position2 - position1;
    if (distanceVec.LengthSquared() > (radius1 + radius2) * (radius1 + radius2)) {
        return; // No collision
    }

    // Axes to test for potential separating planes
    std::vector<Vector3> axes(NUM_AXES, Vector3{});

    // Axes of the first box
    for (int i = 0; i < 3; ++i) {
        axes[i] = obj1->GetAxis(i);
    }

    // Axes of the second box
    for (int i = 0; i < 3; ++i) {
        axes[3 + i] = obj2->GetAxis(i);
    }

    // Edge-edge axes (cross products)
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            Vector3 cross = axes[i].Cross(axes[3 + j]);
            axes[6 + 3 * i + j] = (cross.Normalize());
        }
    }

    float minPenetration = FLT_MAX;
    int minAxisIdx = 0;

    // SAT (Separating Axis Theorem) Test
    for (int i = 0; i < NUM_AXES; ++i) {
        if (axes[i].LengthSquared() <= 0.001f) {
            continue;
        }
        float penetration = CalcPenetration(extents1, extents2, position1, position2, axes, i);
        if (penetration <= 0.f) {
            return; // Separating axis found, no collision
        }
        if (penetration < minPenetration) {
            minPenetration = penetration;
            minAxisIdx = i;
        }
    }
    // Collision normal should point from obj2 to obj1
    Vector3 collisionNormal = axes[minAxisIdx];
    if (collisionNormal.Dot(position1 - position2) < 0) {
        collisionNormal = -collisionNormal;
    }

    // Determine contact points and other collision properties
    CollisionData collisionData;
    collisionData.objects[0] = obj1;
    collisionData.objects[1] = obj2;
    collisionData.collisionNormal = collisionNormal;
    collisionData.penetrationDepth = minPenetration;
    collisionData.restitution = m_objectRestitution;
    collisionData.friction = m_friction;

    CalcContactPointsBoxBox(*box1, *box2, obj1, obj2, collisionData, minAxisIdx, axes);

    AddCollision(collisionData);
}

void Physics::CollisionManager::CheckCollision(Core::Object* obj1, Core::Object* obj2) {
    const Collider* collider1 = obj1->GetCollider();
    const Collider* collider2 = obj2->GetCollider();
    if (collider1 && collider2 && collider1->GetCollisionEnabled() && collider2->GetCollisionEnabled()) {

        if (const auto* sphere1 = dynamic_cast<const SphereCollider*>(collider1)) {
            // Sphere-Box collision
            if (const auto* box2 = dynamic_cast<const BoxCollider*>(collider2)) {
                FindCollisionFeaturesSphereBox(sphere1, box2, obj1, obj2);
            }
            // Sphere-Sphere collision
            else if (const auto* sphere2 = dynamic_cast<const SphereCollider*>(collider2)) {
                FindCollisionFeaturesSphereSphere(sphere1, sphere2, obj1, obj2);
            }
            //// Sphere-Plane collision
            //else if (const auto* plane = dynamic_cast<const PlaneCollider*>(collider2)) {
            //    FindCollisionFeaturesSpherePlane(sphere1, plane, obj1, obj2);
            //}
        }
        else if (const auto* box1 = dynamic_cast<const BoxCollider*>(collider1)) {
            // Box-Sphere collision
            if (const auto* sphere = dynamic_cast<const SphereCollider*>(collider2)) {
                FindCollisionFeaturesSphereBox(sphere, box1, obj2, obj1);
            }
            // Box-Box collision
            else if (const auto* box2 = dynamic_cast<const BoxCollider*>(collider2)) {
                //FindCollisionFeaturesBoxBox(box1, box2, obj1, obj2);
                FindCollisionFeaturesBoxBox(box2, box1, obj2, obj1);
            }
            //// Box-Plane collision
            //else if (const auto* plane = dynamic_cast<const PlaneCollider*>(collider2)) {
            //    FindCollisionFeaturesBoxPlane(box1, plane, obj1, obj2);
            //}
        }
        //else if (const auto* plane = dynamic_cast<const PlaneCollider*>(collider1)) {
        //    //Plane-Box collision
        //    if (const auto* box = dynamic_cast<const BoxCollider*>(collider2)) {
        //        FindCollisionFeaturesBoxPlane(box, plane, obj2, obj1);
        //    }
        //    //Plane-Sphere collision
        //    else if (const auto* sphere = dynamic_cast<const SphereCollider*>(collider2)) {
        //        FindCollisionFeaturesSpherePlane(sphere, plane, obj2, obj1);
        //    }
        //}
    }
}

void Physics::CollisionManager::ResolveCollision(float dt) {
    ThreadPool& pool = ThreadPool::GetInstance();
    m_resolveFutures.clear();

    for (int i = 0; i < m_iterationLimit; ++i) {
        for (auto& contact : m_collisions) {
            // enqueue each collision resolution as a separate task
            m_resolveFutures.push_back(
                pool.enqueue([this, &contact, dt]() {
                    SequentialImpulse(contact, dt);
                    })
            );
        }
    }
    // wait for all collision resolution tasks to complete
    for (auto& future : m_resolveFutures) {
        future.get();
    }
}

float Physics::CollisionManager::ComputeTangentialImpulses(CollisionData& contact, const Vector3& r1, const Vector3& r2, const Vector3& tangent) {
    float inverseMassSum = 0.0f;
    Vector3 termInDenominator1, termInDenominator2;

    // Check if body0 is dynamic and calculate its contributions
    if (contact.objects[0]) {
        const RigidBody* rb = contact.objects[0]->GetRigidBody();
        if (rb) {
            inverseMassSum += rb->GetInverseMass();
            termInDenominator1 = (rb->GetInverseInertiaTensorWorld() * r1.Cross(tangent)).Cross(r1);
        }
    }

    // Check if body1 is dynamic and calculate its contributions
    if (contact.objects[1]) {
        const RigidBody* rb = contact.objects[1]->GetRigidBody();
        if (rb) {
            inverseMassSum += rb->GetInverseMass();
            termInDenominator2 = (rb->GetInverseInertiaTensorWorld() * r2.Cross(tangent)).Cross(r2);
        }
    }

    // Compute the effective mass for the friction/tangential direction
    float effectiveMassTangential = inverseMassSum + (termInDenominator1 + termInDenominator2).Dot(tangent);
    if (effectiveMassTangential == 0.0f) {
        return 0.0f;
    }

    // Calculate relative velocities along the tangent
    Vector3 relativeVel;
    if (contact.objects[0]) {
        const RigidBody* rb = contact.objects[0]->GetRigidBody();
        if (rb) {
            relativeVel += rb->GetLinearVelocity() + rb->GetAngularVelocity().Cross(r1);
        }
    }
    if (contact.objects[1]) {
        const RigidBody* rb = contact.objects[1]->GetRigidBody();
        if (rb) {
            relativeVel -= rb->GetLinearVelocity() + rb->GetAngularVelocity().Cross(r2);
        }
    }

    float relativeSpeedTangential = relativeVel.Dot(tangent);

    // Compute the frictional impulse
    float frictionImpulseMagnitude = -relativeSpeedTangential / effectiveMassTangential;

    // Coulomb's law: The frictional impulse should not be greater than the friction coefficient times the normal impulse
    float maxFriction = contact.friction * contact.accumulatedNormalImpulse;
    frictionImpulseMagnitude = std::clamp(frictionImpulseMagnitude, -maxFriction, maxFriction);

    return frictionImpulseMagnitude;
}

void Physics::CollisionManager::ApplyFrictionImpulses(CollisionData& contact, const Vector3& r1, const Vector3& r2) {

    // Compute the two friction directions
    Vector3 tangent1, tangent2;
    //erin catto - Box2D
    if (abs(contact.collisionNormal.x) >= 0.57735f) {
        tangent1 = Vector3(contact.collisionNormal.y, -contact.collisionNormal.x, 0.0f);
    }
    else {
        tangent1 = Vector3(0.0f, contact.collisionNormal.z, -contact.collisionNormal.y);
    }
    tangent2 = contact.collisionNormal.Cross(tangent1);

    //Compute the impulses in each direction and apply
    float jacobianImpulseT1 = ComputeTangentialImpulses(contact, r1, r2, tangent1);
    ApplyImpulses(contact, jacobianImpulseT1, r1, r2, tangent1);

    float jacobianImpulseT2 = ComputeTangentialImpulses(contact, r1, r2, tangent2);
    ApplyImpulses(contact, jacobianImpulseT2, r1, r2, tangent2);
}

void Physics::CollisionManager::ApplyImpulses(CollisionData& contact, float jacobianImpulse, const Vector3& r1, const Vector3& r2, const Vector3& direction) {
    Vector3 linearImpulse = direction * jacobianImpulse;
    Vector3 angularImpulse1 = r1.Cross(direction) * jacobianImpulse;
    Vector3 angularImpulse2 = r2.Cross(direction) * jacobianImpulse;

    // Check if body0 is dynamic and apply impulse
    if (contact.objects[0]) {
        RigidBody* rb = contact.objects[0]->GetRigidBody();
        if (rb) {
            rb->SetLinearVelocity(
                rb->GetLinearVelocity() + linearImpulse * rb->GetInverseMass()
            );
            rb->SetAngularVelocity(
                rb->GetAngularVelocity() + rb->GetInverseInertiaTensorWorld() * angularImpulse1
            );
        }
    }

    // Check if body1 is dynamic and apply impulse
    if (contact.objects[1]) {
        RigidBody* rb = contact.objects[1]->GetRigidBody();
        if (rb) {
            rb->SetLinearVelocity(
                rb->GetLinearVelocity() - linearImpulse * rb->GetInverseMass()
            );
            rb->SetAngularVelocity(
                rb->GetAngularVelocity() - rb->GetInverseInertiaTensorWorld() * angularImpulse2
            );
        }
    }
}

void Physics::CollisionManager::SequentialImpulse(CollisionData& contact, float deltaTime) {
    // Check if bodies are dynamic and compute the inverse mass sum
    bool isDynamic1 = contact.objects[0]->IsDynamic();
    bool isDynamic2 = contact.objects[1]->IsDynamic();

    RigidBody* rb1 = contact.objects[0]->GetRigidBody();
    RigidBody* rb2 = contact.objects[1]->GetRigidBody();

    float inverseMassSum = isDynamic1 ? rb1->GetInverseMass() : 0.0f;
    if (isDynamic2) {
        inverseMassSum += rb2->GetInverseMass();
    }

    if (inverseMassSum == 0.f) {
        return;
    }

    // Contact point relative to the body's position
    Vector3 r1 = contact.contactPoint.p1.second - (isDynamic1 ? rb1->GetPosition() : contact.objects[0]->GetPosition());
    Vector3 r2 = contact.contactPoint.p2.second - (isDynamic2 ? rb2->GetPosition() : contact.objects[1]->GetPosition());

    // Inverse inertia tensors
    Matrix3 i1 = isDynamic1 ? rb1->GetInverseInertiaTensorWorld() : Matrix3(0.0f);
    Matrix3 i2 = isDynamic2 ? rb2->GetInverseInertiaTensorWorld() : Matrix3(0.0f);

    // Denominator terms
    Vector3 termInDenominator1 = isDynamic1 ? (i1 * r1.Cross(contact.collisionNormal)).Cross(r1) : Vector3(0.0f, 0.0f, 0.0f);
    Vector3 termInDenominator2 = isDynamic2 ? (i2 * r2.Cross(contact.collisionNormal)).Cross(r2) : Vector3(0.0f, 0.0f, 0.0f);

    // Compute the final effective mass
    float effectiveMass =
        inverseMassSum  //linear part
        +
        (termInDenominator1 + termInDenominator2).Dot(contact.collisionNormal); //angular part

    if (effectiveMass == 0.0f) {
        return;
    }

    // Relative velocities
    Vector3 relativeVel = isDynamic1 ? rb1->GetLinearVelocity() + rb1->GetAngularVelocity().Cross(r1) : Vector3(0.0f, 0.0f, 0.0f);
    if (isDynamic2) {
        relativeVel -= (rb2->GetLinearVelocity() + rb2->GetAngularVelocity().Cross(r2));
    }

    float relativeSpeed = relativeVel.Dot(contact.collisionNormal);

    // Baumgarte Stabilization (for penetration & sinking resolution)a
    float baumgarte = 0.0f;
    constexpr float CORRECTION_RATIO = 0.15f;
    if (contact.penetrationDepth > m_penetrationTolerance) {
        baumgarte = ((contact.penetrationDepth - m_penetrationTolerance) * CORRECTION_RATIO / deltaTime);
    }

    float restitutionTerm = 0.0f;
    if (relativeSpeed > m_closingSpeedTolerance) {
        restitutionTerm = contact.restitution * (relativeSpeed - m_closingSpeedTolerance);
    }

    // Compute the impulse
    float jacobianImpulse = ((-(1 + restitutionTerm) * relativeSpeed) + baumgarte) / effectiveMass;

    if (isnan(jacobianImpulse)) {
        return;
    }



    // Clamp the accumulated impulse
    float oldAccumulatedNormalImpulse = contact.accumulatedNormalImpulse;
    contact.accumulatedNormalImpulse = std::max(oldAccumulatedNormalImpulse + jacobianImpulse, 0.0f);
    jacobianImpulse = contact.accumulatedNormalImpulse - oldAccumulatedNormalImpulse;

    // an arbitrary impulse clamping value to maintain stability and realism.
    // Without clamping, might encounter scenarios where objects react unrealistically due to excessively large impulses.
    // causing jittery movements, passing through each other
    // As opposed to solving all collisions simultaneously, current sequential impulse solver requires smaller impulses for each resolution step.
    static constexpr float MAX_IMPULSE = 10.f;
    jacobianImpulse = std::max(std::min(jacobianImpulse, MAX_IMPULSE), -MAX_IMPULSE);

    // Apply impulses to the bodies
    ApplyImpulses(contact, jacobianImpulse, r1, r2, contact.collisionNormal);

    // Compute and apply frictional impulses using the two tangents
    ApplyFrictionImpulses(contact, r1, r2);
}

void Physics::CollisionManager::AddCollision(const CollisionData& data) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_collisions.push_back(data);
}

std::vector<Physics::CollisionData> Physics::CollisionManager::GetCollisions() const{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_collisions;
}

