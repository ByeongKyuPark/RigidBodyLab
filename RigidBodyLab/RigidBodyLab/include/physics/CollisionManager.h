#include <vector>
#include <algorithm>
#include <typeinfo>
#include <memory> // for std::weak_ptr
#include <functional>
#include <future>
#include <physics/CollisionData.h>
#include <physics/Collider.h>

namespace Physics {
    static std::function<bool(float, float)> Less = [](float v1, float v2) { return v1 < v2; };
    static std::function<bool(float, float)> Greater = [](float v1, float v2) { return v1 > v2; };

    using Core::Object;


    class CollisionManager {
    private:
        std::vector<CollisionData> m_collisions;
        std::vector<std::future<void>> m_resolveFutures;
        std::mutex m_collisionResolveMutex;

        float m_friction;
        float m_objectRestitution;
        float m_groundRestitution;

        int m_iterationLimit;
        float m_penetrationTolerance;
        float m_closingSpeedTolerance;

        Vector3 GetBoxContactVertexLocal(const Vector3& axis1, const Vector3& axis2, const Vector3& axis3, Vector3 collisionNormal, std::function<bool(float, float)> cmp) const;

        float CalcPenetration(Vec3& extents1, Vec3& extents2, Vector3& pos1, Vector3& pos2, const std::vector<Vector3>& axes, int axisIdx);

        void CalcContactPointsBoxBox(const BoxCollider& box1, const BoxCollider& box2,
            const Object* obj1, const Object* obj2,
            CollisionData& newContact, int minPenetrationAxisIdx, const std::vector<Vector3>& axes) const;


        // Function to handle Sphere-Box collision
        void FindCollisionFeaturesSphereBox(const SphereCollider* sphere, const BoxCollider* box, Object* sphereObj, Object* boxObj);

        // Function to handle Sphere-Sphere collision
        void FindCollisionFeaturesSphereSphere(const SphereCollider* sphere1, const SphereCollider* sphere2, Object* sphereObj1, Object* sphereObj2) {
            const Vector3& spherePos1 = sphereObj1->GetPosition();
            const Vector3& spherePos2 = sphereObj2->GetPosition();

            float distanceSquared = (spherePos1-spherePos2).LengthSquared();
            float radius1 = std::get<float>(sphere1->GetScale());//std::variant<Vec3, float>
            float radius2 = std::get<float>(sphere2->GetScale());//std::variant<Vec3, float>

            float radiusSum = radius1 + radius2;
            if (distanceSquared > radiusSum * radiusSum) {
                return; // no collision
            }

            Vector3 normal = (spherePos1 - spherePos2).Normalize();

            CollisionData newContact;
            newContact.objects[0] = sphereObj1;
            newContact.objects[1] = sphereObj2;
            newContact.collisionNormal = normal;
            newContact.contactPoint = { { true, Vector3(spherePos1 - normal * radius1) },
                                        { true, Vector3(spherePos2 + normal * radius2) } };
            newContact.penetrationDepth = radiusSum - sqrtf(distanceSquared);
            newContact.restitution = m_objectRestitution;
            newContact.friction = m_friction;
            m_collisions.push_back(newContact);
        }

        // Function to handle Box-Box collision
        void FindCollisionFeaturesBoxBox(
            const BoxCollider* box1, const BoxCollider* box2,
            Object* obj1, Object* obj2);

        // Function to handle Sphere-Plane collision
        //void FindCollisionFeaturesSpherePlane(const SphereCollider* sphere, const PlaneCollider* plane,
        //    Object* obj1, Object* obj2) {
        //    // TODO::Implement Sphere-Plane collision detection logic
        //    return;
        //}
        // Function to handle Box-Plane collision
        //void FindCollisionFeaturesBoxPlane(const BoxCollider* box, const PlaneCollider* plane,
        //    Object* obj1, Object* obj2) {
        //    // TODO::Implement Box-Plane collision detection logic
        //    return;
        //}

    public:
        CollisionManager()
            : m_friction(10.f), m_objectRestitution(0.7f), m_groundRestitution(0.6f),
            m_iterationLimit(3), m_penetrationTolerance(0.0005f), m_closingSpeedTolerance(0.0005f) {}

        void CheckCollision(Core::Object* obj1, Core::Object* obj2);
        void Reset() { m_collisions.clear(); }
        void ResolveCollision(float dt);

        float CollisionManager::ComputeTangentialImpulses(CollisionData& contact, const Vector3& r1, const Vector3& r2, const Vector3& tangent);

        void CollisionManager::ApplyFrictionImpulses(CollisionData& contact, const Vector3& r1, const Vector3& r2);

        void CollisionManager::ApplyImpulses(CollisionData& contact, float jacobianImpulse, const Vector3& r1, const Vector3& r2, const Vector3& direction);
        void CollisionManager::SequentialImpulse(CollisionData& contact, float deltaTime);

    };
}
