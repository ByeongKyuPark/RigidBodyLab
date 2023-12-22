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

        Vector3 GetBoxContactVertexLocal(const Vector3& axis1, const Vector3& axis2, const Vector3& axis3, Vector3 collisionNormal, std::function<bool(float, float)> cmp) const;

        float CalcPenetration(Vec3& extents1, Vec3& extents2, Vector3& pos1, Vector3& pos2, const std::vector<Vector3>& axes, int axisIdx);

        void CalcContactPointsBoxBox(const BoxCollider& box1, const BoxCollider& box2,
            const Object* obj1, const Object* obj2,
            CollisionData& newContact, int minPenetrationAxisIdx, const std::vector<Vector3>& axes) const;


        // Function to handle Sphere-Box collision
        void FindCollisionFeaturesSphereBox(const SphereCollider* sphere, const BoxCollider* box, Object* sphereObj, Object* boxObj);

        // Function to handle Sphere-Sphere collision
        void FindCollisionFeaturesSphereSphere(const SphereCollider* sphere1, const SphereCollider* sphere2,
            Object* obj1, Object* obj2) {
            // Implement Sphere-Sphere collision detection logic here
            return;
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
            : m_friction(0.6f), m_objectRestitution(0.2f), m_groundRestitution(0.2f),
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
