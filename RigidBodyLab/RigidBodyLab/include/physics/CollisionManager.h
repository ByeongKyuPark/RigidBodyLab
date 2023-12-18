#include <vector>
#include <algorithm>
#include <typeinfo>
#include <memory> // for std::weak_ptr
#include <functional>
#include <physics/CollisionData.h>
#include <physics/Collider.h>

namespace Physics {
    static std::function<bool(float, float)> Less = [](float v1, float v2) { return v1 < v2; };
    static std::function<bool(float, float)> Greater = [](float v1, float v2) { return v1 > v2; };

    using Core::Object;


    class CollisionManager {
    private:
        std::vector<CollisionData> m_collisions;

        float m_friction;
        float m_objectRestitution;
        float m_groundRestitution;

        int m_iterationLimit;
        float m_penetrationTolerance;
        float m_closingSpeedTolerance;

        Vector3 GetBoxContactVertexLocal(Vector3 scl, const Vector3 & axis1, const Vector3& axis2, const Vector3& axis3, Vector3 collisionNormal, std::function<bool(float, float)> cmp) const {
            Vector3 contactPoint{ scl.x,  scl.y,  scl.z };

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

        float CalcPenetration(Vec3& extents1, Vec3& extents2, Vector3& pos1, Vector3& pos2,const std::vector<Vector3>& axes, int axisIdx){

            //const BoxCollider* box1, const BoxCollider* box2,
            //const RigidBody* body1, const RigidBody* body2, const Vector3& axis) {

            //if (!box1 || !box2) {
            //    // Handle the error: one of the objects does not have a BoxCollider
            //    return 0.0f;
            //}

            //// Default values for position and orientation in case of static objects
            //Vector3 position1 = body1 ? body1->GetPosition() : Vector3(0, 0, 0);
            //Vector3 position2 = body2 ? body2->GetPosition() : Vector3(0, 0, 0);

            //Vec3 extents1 = std::get<Vec3>(box1->GetScale()); // Assuming GetScale() returns a std::variant<Vec3, float>
            //Vec3 extents2 = std::get<Vec3>(box2->GetScale()); // Assuming GetScale() returns a std::variant<Vec3, float>

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

        void CalcContactPointsBoxBox(const BoxCollider& box1, const BoxCollider& box2,
            const Object& obj1, const Object& obj2,
            CollisionData& newContact, int minPenetrationAxisIdx, const std::vector<Vector3>& axes) const {
            // Check dynamic status of objects
            bool isDynamic1 = obj1.IsDynamic();
            bool isDynamic2 = obj2.IsDynamic();

            // Retrieve transformation matrices or use the RigidBody's transformation
            Mat4 transform1 = isDynamic1 ? obj1.GetRigidBody()->GetLocalToWorldMatrix() : obj1.GetModelRTMatrix();
            Mat4 transform2 = isDynamic2 ? obj2.GetRigidBody()->GetLocalToWorldMatrix() : obj2.GetModelRTMatrix();

            //  	1. for cases 0 to 5, vertices are found to define contact points
            if (minPenetrationAxisIdx >= 0 && minPenetrationAxisIdx < 3)
            {
                Vec3 scl = std::get<Vec3>(box2.GetScale());
                Vector3 contactPoint = GetBoxContactVertexLocal({ scl.x,scl.y,scl.z }, axes[0], axes[1], axes[2] , newContact.collisionNormal, Less);

                Vec4 point = transform2 * Vec4{ contactPoint.x,contactPoint.y,contactPoint.z,1.f };
                contactPoint = { point.x,point.y,point.z };
                newContact.contactPoint = { {true, contactPoint + newContact.collisionNormal * newContact.penetrationDepth},
                                        {true, contactPoint} };
                std::cout << "collision1\n";
            }
            else if (minPenetrationAxisIdx >= 3 && minPenetrationAxisIdx < 6) {
                Vec3 scl = std::get<Vec3>(box1.GetScale());
                Vector3 contactPoint = GetBoxContactVertexLocal({ scl.x,scl.y,scl.z }, axes[3], axes[4], axes[5] , newContact.collisionNormal, Greater);

                Vec4 point = transform1 * Vec4{ contactPoint.x,contactPoint.y,contactPoint.z,1.f };
                contactPoint = { point.x,point.y,point.z };
                newContact.contactPoint = { {true, contactPoint},
                                    {true, contactPoint - newContact.collisionNormal * newContact.penetrationDepth} };
                std::cout << "collision2\n";
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
                Vec3 scl1 = std::get<Vec3>(box1.GetScale());
                Vec3 scl2 = std::get<Vec3>(box2.GetScale());
                Vector3 vertexOne = GetBoxContactVertexLocal({ scl1.x,scl1.y,scl1.z }, axes[0], axes[1], axes[2], newContact.collisionNormal, Greater);
                Vector3 vertexTwo = GetBoxContactVertexLocal({ scl2.x,scl2.y,scl2.z }, axes[3], axes[4], axes[5], newContact.collisionNormal, Less);

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
                Vec4 v1 = obj1.GetModelSRTMatrix() * Vec4 { vertexOne.x, vertexOne.y, vertexOne.z, 1.f };
                vertexOne = { v1.x,v1.y,v1.z };
                Vec4 v2 = obj2.GetModelSRTMatrix() * Vec4 { vertexTwo.x, vertexTwo.y, vertexTwo.z, 1.f };
                vertexTwo = { v2.x,v2.y,v2.z };

                //1. calculate the dot product between edge1 and edge2:
                float k = edge1.Dot(edge2);//cosine

                //2.  point on the edge of box1 closest to the initial contact point on box2
                //    The calculation involves projecting the vector from contactPoint1 to contactPoint2 onto the direction of edge1 - edge2 * k.
                Vector3 closestPointOne = { vertexOne + edge1 * ((vertexTwo - vertexOne).Dot(edge1 - edge2 * k) / (1 - k * k)) };

                //3. point on the edge of box2 closest to 
                //projecting the vector from closestPointOne to vertexTwo onto direction2.
                Vector3 closestPointTwo{ vertexTwo + edge2 * ((closestPointOne - vertexTwo).Dot(edge2)) };
                std::cout << "collision3\n";
                newContact.contactPoint = { { true, closestPointOne },
                                        { true, closestPointTwo } };
            }
        }


        // Function to handle Sphere-Box collision
        void FindCollisionFeaturesSphereBox(const SphereCollider* sphere, const BoxCollider* box,
            const Object& obj1, const Object& obj2) {
            // Determine if objects are dynamic and retrieve position and axes
            bool isDynamic1 = obj1.IsDynamic();
            bool isDynamic2 = obj2.IsDynamic();

            Vector3 position1 = isDynamic1 ? obj1.GetRigidBody()->GetPosition() : Vector3(obj1.GetModelRTMatrix()[3].x, obj1.GetModelRTMatrix()[3].y, obj1.GetModelRTMatrix()[3].z);
            Vector3 position2 = isDynamic2 ? obj2.GetRigidBody()->GetPosition() : Vector3(obj2.GetModelRTMatrix()[3].x, obj2.GetModelRTMatrix()[3].y, obj2.GetModelRTMatrix()[3].z);

            constexpr int NUM_AXES = 3;
            Vec3 extents = std::get<Vec3>(box->GetScale());//std::variant<Vec3, float>

            Vector3 closestPoint = position1;
            Vector3 centerToCenter = position2 - position1;

            // For each axis (X, Y, Z)
            for (int i = 0; i < NUM_AXES; ++i) {
                Vector3 axis = isDynamic1 ? obj1.GetRigidBody()->GetAxis(i) : Vector3(obj1.GetModelRTMatrix()[i].x, obj1.GetModelRTMatrix()[i].y, obj1.GetModelRTMatrix()[i].z);
                //Vector3 axis = body1->GetAxis(i);
                float extent = extents[i]; // Directly access the extents array

                float projectionLength = centerToCenter.Dot(axis);
                projectionLength = std::clamp(projectionLength, -extent, extent);
                closestPoint += axis * projectionLength;
            }

            // Calculate squared distance to sphere center
            float distanceSquared = (closestPoint - position2).LengthSquared();
            float radius = std::get<float>(sphere->GetScale());//std::variant<Vec3, float>

            if (distanceSquared > radius * radius) {
                return; // No collision
            }

            // If a collision is detected, populate and return CollisionData
            CollisionData collisionData;
            collisionData.bodies[0] = isDynamic1 ? const_cast<Physics::RigidBody*>(obj1.GetRigidBody()) : nullptr;
            collisionData.bodies[1] = isDynamic2 ? const_cast<Physics::RigidBody*>(obj2.GetRigidBody()) : nullptr;
            collisionData.collisionNormal = position2 - closestPoint;
            collisionData.collisionNormal.Normalize();
            collisionData.contactPoint = {
                { true, Vector3(position1 - collisionData.collisionNormal * radius) },
                { true, Vector3(closestPoint) }
            };

            collisionData.penetrationDepth = radius - std::sqrt(distanceSquared);
            collisionData.restitution = m_objectRestitution;
            collisionData.friction = m_friction;
            m_collisions.push_back(collisionData);
        }

        // Function to handle Sphere-Sphere collision
        void FindCollisionFeaturesSphereSphere(const SphereCollider* sphere1, const SphereCollider* sphere2,
            const Object& obj1, const Object& obj2) {
            // Implement Sphere-Sphere collision detection logic here
            return;
        }

        // Function to handle Box-Box collision
        void FindCollisionFeaturesBoxBox(
            const BoxCollider* box1, const BoxCollider* box2,
            const Object& obj1, const Object& obj2) {

            static constexpr int NUM_AXES = 15;

            bool isDynamic1 = obj1.IsDynamic();
            bool isDynamic2 = obj2.IsDynamic();

            Vec3 extents1 = std::get<Vec3>(box1->GetScale());
            Vec3 extents2 = std::get<Vec3>(box2->GetScale());

            float radius1 = std::max({ extents1.x, extents1.y, extents1.z });
            float radius2 = std::max({ extents2.x, extents2.y, extents2.z });

            Vector3 position1 = isDynamic1 ? obj1.GetRigidBody()->GetPosition() : Vector3(obj1.GetModelRTMatrix()[3].x, obj1.GetModelRTMatrix()[3].y, obj1.GetModelRTMatrix()[3].z);
            Vector3 position2 = isDynamic2 ? obj2.GetRigidBody()->GetPosition() : Vector3(obj2.GetModelRTMatrix()[3].x, obj2.GetModelRTMatrix()[3].y, obj2.GetModelRTMatrix()[3].z);
            //std::cout << position1 << std::endl;
            //std::cout << position2 << std::endl;

            Vector3 distanceVec = position2 - position1;
            if (distanceVec.LengthSquared() > (radius1 + radius2) * (radius1 + radius2)) {
                return; // No collision
            }

            // Axes to test for potential separating planes
            std::vector<Vector3> axes(NUM_AXES, Vector3{});

            // Axes of the first box
            for (int i = 0; i < 3; ++i) {
                axes[i]=(isDynamic1 ? obj1.GetRigidBody()->GetAxis(i) : Vector3(obj1.GetModelRTMatrix()[i].x, obj1.GetModelRTMatrix()[i].y, obj1.GetModelRTMatrix()[i].z));
                axes[i]=axes[i].Normalize();
            }

            // Axes of the second box
            for (int i = 0; i < 3; ++i) {
                axes[3+i]=(isDynamic2 ? obj2.GetRigidBody()->GetAxis(i) : Vector3(obj2.GetModelRTMatrix()[i].x, obj2.GetModelRTMatrix()[i].y, obj2.GetModelRTMatrix()[i].z));
                axes[3+i] = axes[3+i].Normalize();
            }

            // Edge-edge axes (cross products)
            for (int i = 0; i < 3; ++i) {
                for (int j = 0; j < 3; ++j) {
                    Vector3 cross = axes[i].Cross(axes[3 + j]);
                    if (cross.LengthSquared() > 0.001f) {
                        axes[6+3*i+j]=(cross.Normalize());
                    }
                }
            }

            float minPenetration = FLT_MAX;
            int minAxisIdx = 0;

            // SAT (Separating Axis Theorem) Test
            for (int i = 0; i < NUM_AXES; ++i) {
                if (axes[i].LengthSquared() <= 0.001f) {
                    continue;
                }
                float penetration = CalcPenetration(extents1, extents2,position1,position2, axes,i);
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
            collisionData.bodies[0] = isDynamic1 ? const_cast<Physics::RigidBody*>(obj1.GetRigidBody()) : nullptr;
            collisionData.bodies[1] = isDynamic2 ? const_cast<Physics::RigidBody*>(obj2.GetRigidBody()) : nullptr;
            collisionData.collisionNormal = collisionNormal;
            collisionData.penetrationDepth = minPenetration;
            collisionData.restitution = m_objectRestitution;
            collisionData.friction = m_friction;

            CalcContactPointsBoxBox(*box1, *box2, obj1, obj2, collisionData, minAxisIdx,axes);

            m_collisions.push_back(collisionData);
        }

        // Function to handle Sphere-Plane collision
        void FindCollisionFeaturesSpherePlane(const SphereCollider* sphere, const PlaneCollider* plane,
            const Object& obj1, const Object& obj2) {
            // TODO::Implement Sphere-Plane collision detection logic
            return;
        }
        // Function to handle Box-Plane collision
        void FindCollisionFeaturesBoxPlane(const BoxCollider* box, const PlaneCollider* plane,
            const Object& obj1, const Object& obj2) {
            // TODO::Implement Box-Plane collision detection logic
            return;
        }

    public:
        CollisionManager()
            : m_friction(0.6f), m_objectRestitution(0.5f), m_groundRestitution(0.2f),
            m_iterationLimit(1), m_penetrationTolerance(0.0005f), m_closingSpeedTolerance(0.005f) {}

        void CheckCollision(const Core::Object& obj1, const Core::Object& obj2) {
            const Collider* collider1 = obj1.GetCollider();
            const Collider* collider2 = obj2.GetCollider();

            if (const auto* sphere1 = dynamic_cast<const SphereCollider*>(collider1)) {
                // Sphere-Box collision
                if (const auto* box2 = dynamic_cast<const BoxCollider*>(collider2)) {
                    FindCollisionFeaturesSphereBox(sphere1, box2, obj1, obj2);
                }
                // Sphere-Sphere collision
                else if (const auto* sphere2 = dynamic_cast<const SphereCollider*>(collider2)) {
                    FindCollisionFeaturesSphereSphere(sphere1, sphere2, obj1, obj2);
                }
                // Sphere-Plane collision
                else if (const auto* plane = dynamic_cast<const PlaneCollider*>(collider2)) {
                    FindCollisionFeaturesSpherePlane(sphere1, plane, obj1, obj2);
                }
            }
            else if (const auto* box1 = dynamic_cast<const BoxCollider*>(collider1)) {
                // Box-Sphere collision
                if (const auto* sphere = dynamic_cast<const SphereCollider*>(collider2)) {
                    FindCollisionFeaturesSphereBox(sphere, box1, obj2, obj1);
                }
                // Box-Box collision
                else if (const auto* box2 = dynamic_cast<const BoxCollider*>(collider2)) {
                    FindCollisionFeaturesBoxBox(box1, box2, obj1, obj2);
                }
                // Box-Plane collision
                else if (const auto* plane = dynamic_cast<const PlaneCollider*>(collider2)) {
                    FindCollisionFeaturesBoxPlane(box1, plane, obj1, obj2);
                }
            }
            else if (const auto* plane = dynamic_cast<const PlaneCollider*>(collider1)) {
                //Plane-Box collision
                if (const auto* box = dynamic_cast<const BoxCollider*>(collider2)) {
                    FindCollisionFeaturesBoxPlane(box, plane, obj2, obj1);
                }
                //Plane-Sphere collision
                else if (const auto* sphere = dynamic_cast<const SphereCollider*>(collider2)) {
                    FindCollisionFeaturesSpherePlane(sphere, plane, obj2, obj1);
                }
            }

        }
        void Clear() { m_collisions.clear(); }
        void ResolveCollision(float dt) {
            for (int i = 0; i < m_iterationLimit; ++i) {
                for (auto& contact : m_collisions) {
                    SequentialImpulse(contact, dt);
                }
            }
        }

        float CollisionManager::ComputeTangentialImpulses(CollisionData& contact, const Vector3& r1, const Vector3& r2, const Vector3& tangent) {
            float inverseMassSum = 0.0f;
            Vector3 termInDenominator1, termInDenominator2;

            // Check if body0 is dynamic and calculate its contributions
            if (contact.bodies[0]) {
                inverseMassSum += contact.bodies[0]->GetInverseMass();
                termInDenominator1 = (contact.bodies[0]->GetInverseInertiaTensorWorld() * r1.Cross(tangent)).Cross(r1);
            }

            // Check if body1 is dynamic and calculate its contributions
            if (contact.bodies[1]) {
                inverseMassSum += contact.bodies[1]->GetInverseMass();
                termInDenominator2 = (contact.bodies[1]->GetInverseInertiaTensorWorld() * r2.Cross(tangent)).Cross(r2);
            }

            // Compute the effective mass for the friction/tangential direction
            float effectiveMassTangential = inverseMassSum + (termInDenominator1 + termInDenominator2).Dot(tangent);
            if (effectiveMassTangential == 0.0f) {
                return 0.0f;
            }

            // Calculate relative velocities along the tangent
            Vector3 relativeVel;
            if (contact.bodies[0]) {
                relativeVel += contact.bodies[0]->GetLinearVelocity() + contact.bodies[0]->GetAngularVelocity().Cross(r1);
            }
            if (contact.bodies[1]) {
                relativeVel -= contact.bodies[1]->GetLinearVelocity() + contact.bodies[1]->GetAngularVelocity().Cross(r2);
            }

            float relativeSpeedTangential = relativeVel.Dot(tangent);

            // Compute the frictional impulse
            float frictionImpulseMagnitude = -relativeSpeedTangential / effectiveMassTangential;

            // Coulomb's law: The frictional impulse should not be greater than the friction coefficient times the normal impulse
            float maxFriction = contact.friction * contact.accumulatedNormalImpulse;
            frictionImpulseMagnitude = std::clamp(frictionImpulseMagnitude, -maxFriction, maxFriction);

            return frictionImpulseMagnitude;
        }

        void CollisionManager::ApplyFrictionImpulses(CollisionData& contact, const Vector3& r1, const Vector3& r2) {

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

            // Compute the impulses in each direction and apply
            //float jacobianImpulseT1 = ComputeTangentialImpulses(contact, r1, r2, tangent1);
            //ApplyImpulses(contact, jacobianImpulseT1, r1, r2, tangent1);

            //float jacobianImpulseT2 = ComputeTangentialImpulses(contact, r1, r2, tangent2);
            //ApplyImpulses(contact, jacobianImpulseT2, r1, r2, tangent2);
        }

        void CollisionManager::ApplyImpulses(CollisionData& contact, float jacobianImpulse, const Vector3& r1, const Vector3& r2, const Vector3& direction) {
            Vector3 linearImpulse = direction * jacobianImpulse;
            Vector3 angularImpulse1 = r1.Cross(direction) * jacobianImpulse;
            Vector3 angularImpulse2 = r2.Cross(direction) * jacobianImpulse;

            // Check if body0 is dynamic and apply impulse
            if (contact.bodies[0]) {
                contact.bodies[0]->SetLinearVelocity(
                    contact.bodies[0]->GetLinearVelocity() + linearImpulse * contact.bodies[0]->GetInverseMass()
                );
                contact.bodies[0]->SetAngularVelocity(
                    contact.bodies[0]->GetAngularVelocity() + contact.bodies[0]->GetInverseInertiaTensorWorld() * angularImpulse1
                );
                if (std::isnan(contact.bodies[0]->GetLinearVelocity().x) || std::isnan(contact.bodies[0]->GetLinearVelocity().y) || std::isnan(contact.bodies[0]->GetLinearVelocity().z)) {
                    std::cerr << "NaN detected in position\n";
                }
            }


            // Check if body1 is dynamic and apply impulse
            if (contact.bodies[1]) {
                contact.bodies[1]->SetLinearVelocity(
                    contact.bodies[1]->GetLinearVelocity() - linearImpulse * contact.bodies[1]->GetInverseMass()
                );
                contact.bodies[1]->SetAngularVelocity(
                    contact.bodies[1]->GetAngularVelocity() - contact.bodies[1]->GetInverseInertiaTensorWorld() * angularImpulse2
                );
                if (std::isnan(contact.bodies[1]->GetLinearVelocity().x) || std::isnan(contact.bodies[1]->GetLinearVelocity().y) || std::isnan(contact.bodies[1]->GetLinearVelocity().z)) {
                    std::cerr << "NaN detected in position\n";
                }
            }
        }

        void CollisionManager::SequentialImpulse(CollisionData& contact, float deltaTime) {
            // Check if bodies are dynamic and compute the inverse mass sum
            bool isDynamic1 = contact.bodies[0] != nullptr;
            bool isDynamic2 = contact.bodies[1] != nullptr;

            float inverseMassSum = isDynamic1 ? contact.bodies[0]->GetInverseMass() : 0.0f;
            if (isDynamic2) {
                inverseMassSum += contact.bodies[1]->GetInverseMass();
            }

            if (inverseMassSum == 0.f) {
                return;
            }

            // Contact point relative to the body's position
            Vector3 r1 = isDynamic1 ? contact.contactPoint.p1.second - contact.bodies[0]->GetPosition() : Vector3(0.0f, 0.0f, 0.0f);
            Vector3 r2 = isDynamic2 ? contact.contactPoint.p2.second - contact.bodies[1]->GetPosition() : Vector3(0.0f, 0.0f, 0.0f);

            // Inverse inertia tensors
            Matrix3 i1 = isDynamic1 ? contact.bodies[0]->GetInverseInertiaTensorWorld() : Matrix3(0.0f);
            Matrix3 i2 = isDynamic2 ? contact.bodies[1]->GetInverseInertiaTensorWorld() : Matrix3(0.0f);

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
            Vector3 relativeVel = isDynamic1 ? contact.bodies[0]->GetLinearVelocity() + contact.bodies[0]->GetAngularVelocity().Cross(r1) : Vector3(0.0f, 0.0f, 0.0f);
            if (isDynamic2) {
                relativeVel -= (contact.bodies[1]->GetLinearVelocity() + contact.bodies[1]->GetAngularVelocity().Cross(r2));
            }

            float relativeSpeed = relativeVel.Dot(contact.collisionNormal);

            // Baumgarte Stabilization (for penetration & sinking resolution)
            float baumgarte = 0.0f;
            constexpr float CORRECTION_RATIO = 0.1f;
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

            // Apply impulses to the bodies
            ApplyImpulses(contact, jacobianImpulse, r1, r2, contact.collisionNormal);

            // Compute and apply frictional impulses using the two tangents
            ApplyFrictionImpulses(contact, r1, r2);
        }

    };
}
