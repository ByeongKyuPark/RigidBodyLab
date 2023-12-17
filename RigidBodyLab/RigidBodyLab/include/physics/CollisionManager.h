#include <vector>
#include <algorithm>
#include <typeinfo>
#include <memory> // for std::weak_ptr
#include <physics/CollisionData.h>
#include <physics/Collider.h>

namespace Physics {
    class CollisionManager {
    private:
        std::vector<CollisionData> m_collisions;

        float m_friction;
        float m_objectRestitution;
        float m_groundRestitution;

        int m_iterationLimit;
        float m_penetrationTolerance;
        float m_closingSpeedTolerance;

        float CalcPenetration(const BoxCollider* box1, const BoxCollider* box2,
            const std::shared_ptr<RigidBody>& body1, const std::shared_ptr<RigidBody>& body2,
            const Vector3& axis) {

            if (!box1 || !box2) {
                // Handle the error: one of the objects does not have a BoxCollider
                return 0.0f;
            }

            // Default values for position and orientation in case of static objects
            Vector3 position1 = body1 ? body1->GetPosition() : Vector3(0, 0, 0);
            Vector3 position2 = body2 ? body2->GetPosition() : Vector3(0, 0, 0);

            Vec3 extents1 = std::get<Vec3>(box1->GetScale()); // Assuming GetScale() returns a std::variant<Vec3, float>
            Vec3 extents2 = std::get<Vec3>(box2->GetScale()); // Assuming GetScale() returns a std::variant<Vec3, float>

            Vector3 centerToCenter = position2 - position1;
            float projectedCenterToCenter = abs(centerToCenter.Dot(axis));

            // Use default axes (i.e., world axes) if no RigidBody is present
            Vector3 axes1[3] = { Vector3(1, 0, 0), Vector3(0, 1, 0), Vector3(0, 0, 1) };
            Vector3 axes2[3] = { Vector3(1, 0, 0), Vector3(0, 1, 0), Vector3(0, 0, 1) };

            if (body1) {
                for (int i = 0; i < 3; ++i) axes1[i] = body1->GetAxis(i);
            }
            if (body2) {
                for (int i = 0; i < 3; ++i) axes2[i] = body2->GetAxis(i);
            }

            float projectedSum =
                abs((axes1[0] * extents1.x).Dot(axis)) +
                abs((axes1[1] * extents1.y).Dot(axis)) +
                abs((axes1[2] * extents1.z).Dot(axis)) +

                abs((axes2[0] * extents2.x).Dot(axis)) +
                abs((axes2[1] * extents2.y).Dot(axis)) +
                abs((axes2[2] * extents2.z).Dot(axis));

            return projectedSum - projectedCenterToCenter;
        }

        // Function to handle Sphere-Box collision
        void FindCollisionFeaturesSphereBox(const SphereCollider* sphere, const BoxCollider* box,
            const std::shared_ptr<RigidBody>& body1, const std::shared_ptr<RigidBody>& body2) {
            constexpr int NUM_AXES = 3;
            Vector3 centerToCenter = body2->GetPosition() - body1->GetPosition();
            Vec3 extents = std::get<Vec3>(box->GetScale());//std::variant<Vec3, float>

            Vector3 closestPoint = body1->GetPosition();

            // For each axis (X, Y, Z)
            for (int i = 0; i < NUM_AXES; ++i) {
                Vector3 axis = body1->GetAxis(i);
                float extent = extents[i]; // Directly access the extents array

                float projectionLength = centerToCenter.Dot(axis);
                projectionLength = std::clamp(projectionLength, -extent, extent);
                closestPoint += axis * projectionLength;
            }

            // Calculate squared distance to sphere center
            float distanceSquared = (closestPoint - body2->GetPosition()).LengthSquared();
            float radius = std::get<float>(sphere->GetScale());//std::variant<Vec3, float>

            if (distanceSquared > radius * radius) {
                return; // No collision
            }

            // If a collision is detected, populate and return CollisionData
            CollisionData collisionData;
            collisionData.bodies[0] = body1;
            collisionData.bodies[1] = body2;
            collisionData.collisionNormal = body2->GetPosition() - closestPoint;
            collisionData.collisionNormal.Normalize();
            collisionData.contactPoint = closestPoint;
            collisionData.penetrationDepth = radius - std::sqrt(distanceSquared);
            collisionData.restitution = m_objectRestitution;
            collisionData.friction = m_friction;
            m_collisions.push_back(collisionData);
        }

        // Function to handle Sphere-Sphere collision
        void FindCollisionFeaturesSphereSphere(const SphereCollider* sphere1, const SphereCollider* sphere2,
            const std::shared_ptr<RigidBody>& body1, const std::shared_ptr<RigidBody>& body2) {
            // Implement Sphere-Sphere collision detection logic here
            return;
        }

        // Function to handle Box-Box collision
        void FindCollisionFeaturesBoxBox(
            const BoxCollider* box1, const BoxCollider* box2,
            const std::shared_ptr<RigidBody>& body1, const std::shared_ptr<RigidBody>& body2) {

            // Broad phase check using sphere bounding (optional for efficiency)
            Vec3 extents1 = std::get<Vec3>(box1->GetScale());//std::variant<Vec3, float>
            Vec3 extents2 = std::get<Vec3>(box2->GetScale());//std::variant<Vec3, float>

            float radius1 = extents1.x > extents1.y ? (extents1.x > extents1.z ? extents1.x : extents1.z) : (extents1.y > extents1.z ? extents1.y : extents1.z);
            float radius2 = extents2.x > extents2.y ? (extents2.x > extents2.z ? extents2.x : extents2.z) : (extents2.y > extents2.z ? extents2.y : extents2.z);
            Vector3 distanceVec = body2->GetPosition() - body1->GetPosition();
            if (distanceVec.LengthSquared() > (radius1 + radius2) * (radius1 + radius2)) {
                return;
            }

            // Axes to test for potential separating planes
            std::vector<Vector3> axes;

            // Axes of the first box
            for (int i = 0; i < 3; ++i) {
                axes.push_back(body1->GetAxis(i));
            }

            // Axes of the second box
            for (int i = 0; i < 3; ++i) {
                axes.push_back(body2->GetAxis(i));
            }

            // Edge-edge axes (cross products)
            for (int i = 0; i < 3; ++i) {
                for (int j = 0; j < 3; ++j) {
                    Vector3 cross = axes[i].Cross(axes[3 + j]);
                    if (cross.LengthSquared() > 0.001f) { // To avoid near-zero length vectors
                        axes.push_back(cross.Normalize());
                    }
                }
            }

            float minPenetration = FLT_MAX;
            Vector3 minAxis;
            for (const auto& axis : axes) {
                float penetration = CalcPenetration(box1, box2, body1, body2, axis);
                if (penetration <= 0) {
                    return; // Separating axis found, no collision
                }
                if (penetration < minPenetration) {
                    minPenetration = penetration;
                    minAxis = axis;
                }
            }

            // Collision normal should point from body2 to body1
            Vector3 collisionNormal = minAxis;
            if (collisionNormal.Dot(body2->GetPosition() - body1->GetPosition()) < 0) {
                collisionNormal = -collisionNormal;
            }

            // Determine contact points and other collision properties
            CollisionData collisionData;
            collisionData.bodies[0] = body1;
            collisionData.bodies[1] = body2;
            collisionData.collisionNormal = collisionNormal;
            collisionData.penetrationDepth = minPenetration;
            // Set restitution and friction based on material properties of bodies
            //collisionData.restitution = CalculateRestitution(body1, body2);
            //collisionData.friction = CalculateFriction(body1, body2);
            collisionData.restitution = m_objectRestitution;
            collisionData.friction = m_friction;

            //TODO:: Calculate contact points...

            m_collisions.push_back(collisionData);
        }

        // Function to handle Sphere-Plane collision
        void FindCollisionFeaturesSpherePlane(const SphereCollider* sphere, const PlaneCollider* plane,
            const std::shared_ptr<RigidBody>& body1) {
            // Implement Sphere-Plane collision detection logic here
            return;
        }
        // Function to handle Box-Plane collision
        void FindCollisionFeaturesSpherePlane(const BoxCollider* box, const PlaneCollider* plane,
            const std::shared_ptr<RigidBody>& body1) {
            // Implement Box-Plane collision detection logic here
            return;
        }
    public:
        CollisionManager()
            : m_friction(0.6f), m_objectRestitution(0.5f), m_groundRestitution(0.2f),
            m_iterationLimit(30), m_penetrationTolerance(0.0005f), m_closingSpeedTolerance(0.005f) {}

        void CheckCollision(const Core::Object& obj1, const Core::Object& obj2) {
            const Collider* collider1 = obj1.GetCollider();
            const Collider* collider2 = obj2.GetCollider();

            std::shared_ptr<RigidBody> body1 = obj1.GetRigidBody();
            std::shared_ptr<RigidBody> body2 = obj2.GetRigidBody();

            // Sphere-Box collision
            if (const SphereCollider* sphere1 = dynamic_cast<const SphereCollider*>(collider1)) {
                if (const BoxCollider* box2 = dynamic_cast<const BoxCollider*>(collider2)) {
                    FindCollisionFeaturesSphereBox(sphere1, box2, body1, body2);
                }
            }
            // Box-Sphere collision
            else if (const BoxCollider* box1 = dynamic_cast<const BoxCollider*>(collider1)) {
                if (const SphereCollider* sphere2 = dynamic_cast<const SphereCollider*>(collider2)) {
                    FindCollisionFeaturesSphereBox(sphere2, box1, body2, body1);
                }
            }
            // Box-Box collision
            else if (const BoxCollider* box1 = dynamic_cast<const BoxCollider*>(collider1)) {
                if (const BoxCollider* box2 = dynamic_cast<const BoxCollider*>(collider2)) {
                    FindCollisionFeaturesBoxBox(box1, box2, body1, body2);
                }
            }
            // Sphere-Sphere collision
            else if (const SphereCollider* sphere1 = dynamic_cast<const SphereCollider*>(collider1)) {
                if (const SphereCollider* sphere2 = dynamic_cast<const SphereCollider*>(collider2)) {
                    FindCollisionFeaturesSphereSphere(sphere1, sphere2, body1, body2);
                }
            }
            // Sphere-Plane collision
            else if (const SphereCollider* sphere = dynamic_cast<const SphereCollider*>(collider1)) {
                if (const PlaneCollider* plane = dynamic_cast<const PlaneCollider*>(collider2)) {
                    FindCollisionFeaturesSpherePlane(sphere, plane, body1);
                }
            }
            // Box-Plane collision
            else if (const BoxCollider* box = dynamic_cast<const BoxCollider*>(collider1)) {
                if (const PlaneCollider* plane = dynamic_cast<const PlaneCollider*>(collider2)) {
                    FindCollisionFeaturesSpherePlane(box, plane, body1);
                }
            }
        }
        void Clear() { m_collisions.clear(); }
        void ResolveCollision() {

        }
    };
}
