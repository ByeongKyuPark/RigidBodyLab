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
        std::vector<CollisionData> collisions;

        // Function to handle Sphere-Box collision
        std::optional<CollisionData> FindCollisionFeaturesSphereBox(const SphereCollider* sphere, const BoxCollider* box,
            const std::shared_ptr<RigidBody>& body1, const std::shared_ptr<RigidBody>& body2) {
            // Implement Sphere-Box collision detection logic here
            return std::nullopt;
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
            for (auto& collision : collisions) {
                // Resolve each collision
                // Example: adjust positions, apply impulses, etc.
            }
        }
    };
}
