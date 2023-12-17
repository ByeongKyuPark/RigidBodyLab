#include <vector>
#include <optional>
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

        // Function to handle Sphere-Box collision
        std::optional<CollisionData> FindCollisionFeaturesSphereBox(const SphereCollider* sphere, const BoxCollider* box,
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
                return std::nullopt; // No collision
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
			return collisionData;
        }

        // Function to handle Sphere-Sphere collision
        std::optional<CollisionData> FindCollisionFeaturesSphereSphere(const SphereCollider* sphere1, const SphereCollider* sphere2,
            const std::shared_ptr<RigidBody>& body1, const std::shared_ptr<RigidBody>& body2) {
            // Implement Sphere-Sphere collision detection logic here
            return std::nullopt;
        }

        // Function to handle Box-Box collision
        std::optional<CollisionData> FindCollisionFeaturesBoxBox(const BoxCollider* box1, const BoxCollider* box2,
            const std::shared_ptr<RigidBody>& body1, const std::shared_ptr<RigidBody>& body2) {
            // Implement Box-Box collision detection logic here
            return std::nullopt;
        }


    public:
        CollisionManager()
            : m_friction(0.6f), m_objectRestitution(0.5f), m_groundRestitution(0.2f),
            m_iterationLimit(30), m_penetrationTolerance(0.0005f), m_closingSpeedTolerance(0.005f) {}

        std::optional<CollisionData> CheckCollision(const Core::Object& obj1, const Core::Object& obj2) {
            const Collider* collider1 = obj1.GetCollider();
            const Collider* collider2 = obj2.GetCollider();

            std::shared_ptr<RigidBody> body1 = obj1.GetRigidBody();
            std::shared_ptr<RigidBody> body2 = obj2.GetRigidBody();

            // Sphere-Box collision
            if (const SphereCollider* sphere1 = dynamic_cast<const SphereCollider*>(collider1)) {
                if (const BoxCollider* box2 = dynamic_cast<const BoxCollider*>(collider2)) {
                    return FindCollisionFeaturesSphereBox(sphere1, box2, body1, body2);
                }
            }
            // Box-Sphere collision
            else if (const BoxCollider* box1 = dynamic_cast<const BoxCollider*>(collider1)) {
                if (const SphereCollider* sphere2 = dynamic_cast<const SphereCollider*>(collider2)) {
                    return FindCollisionFeaturesSphereBox(sphere2, box1, body2, body1);
                }
            }
            // Box-Box collision
            else if (const BoxCollider* box1 = dynamic_cast<const BoxCollider*>(collider1)) {
                if (const BoxCollider* box2 = dynamic_cast<const BoxCollider*>(collider2)) {
                    return FindCollisionFeaturesBoxBox(box1, box2, body1, body2);
                }
            }
            // Sphere-Sphere collision
            else if (const SphereCollider* sphere1 = dynamic_cast<const SphereCollider*>(collider1)) {
                if (const SphereCollider* sphere2 = dynamic_cast<const SphereCollider*>(collider2)) {
                    return FindCollisionFeaturesSphereSphere(sphere1, sphere2, body1, body2);
                }
            }

            return std::nullopt; // No collision detected
        }
        void ResolveCollisions() {
            for (auto& collision : m_collisions) {
                // Resolve each collision
                // Example: adjust positions, apply impulses, etc.
            }
        }
    };
}
