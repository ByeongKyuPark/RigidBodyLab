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


        Vector3 GetBoxContactVertexLocal(Vector3 scl, std::array<Vector3, 3>& axes, Vector3 collisionNormal, std::function<bool(float, float)> cmp) const {
            Vector3 contactPoint{ scl.x,  scl.y,  scl.z };

            if (cmp(axes[0].Dot(collisionNormal), 0)) {
                contactPoint.x *= -1.0f;
            }
            if (cmp(axes[1].Dot(collisionNormal), 0)) {
                contactPoint.y *= -1.0f;
            }
            if (cmp(axes[2].Dot(collisionNormal), 0)) {
                contactPoint.z *= -1.0f;
            }
            return contactPoint;
        }

        float CalcPenetration(const BoxCollider* box1, const BoxCollider* box2,
            const RigidBody* body1, const RigidBody* body2, const Vector3& axis) {

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

        void CalcContactPointsBoxBox(const BoxCollider& box1, const BoxCollider& box2,
            const Object& obj1, const Object& obj2,
            CollisionData& newContact, int minPenetrationAxisIdx) const {
            // Check dynamic status of objects
            bool isDynamic1 = obj1.IsDynamic();
            bool isDynamic2 = obj2.IsDynamic();

            // Retrieve transformation matrices or use the RigidBody's transformation
            Mat4 transform1 = isDynamic1 ? obj1.GetRigidBody()->GetLocalToWorldMatrix() : obj1.GetModelRTMatrix();
            Mat4 transform2 = isDynamic2 ? obj2.GetRigidBody()->GetLocalToWorldMatrix() : obj2.GetModelRTMatrix();

            // Retrieve axes from either the RigidBody or the transformation matrix
            std::array<Vector3, 3> axes1, axes2;
            for (int i = 0; i < 3; ++i) {
                axes1[i] = isDynamic1 ? obj1.GetRigidBody()->GetAxis(i) : Vector3(transform1[i].x, transform1[i].y, transform1[i].z);
                axes2[i] = isDynamic2 ? obj2.GetRigidBody()->GetAxis(i) : Vector3(transform2[i].x, transform2[i].y, transform2[i].z);
            }

            //  	1. for cases 0 to 5, vertices are found to define contact points
            if (minPenetrationAxisIdx >= 0 && minPenetrationAxisIdx < 3)
            {
                Vec3 scl = std::get<Vec3>(box2.GetScale());
                Vector3 contactPoint = GetBoxContactVertexLocal({ scl.x,scl.y,scl.z }, axes2, newContact.collisionNormal, Less);

                Vec4 point = transform2 * Vec4{ contactPoint.x,contactPoint.y,contactPoint.z,1.f };
                contactPoint = { point.x,point.y,point.z };
                newContact.contactPoint = { {true, contactPoint + newContact.collisionNormal * newContact.penetrationDepth},
                                        {true, contactPoint} };
            }
            else if (minPenetrationAxisIdx >= 3 && minPenetrationAxisIdx < 6) {
                Vec3 scl = std::get<Vec3>(box1.GetScale());
                Vector3 contactPoint = GetBoxContactVertexLocal({ scl.x,scl.y,scl.z }, axes1, newContact.collisionNormal, Greater);

                Vec4 point = transform1 * Vec4{ contactPoint.x,contactPoint.y,contactPoint.z,1.f };
                contactPoint = { point.x,point.y,point.z };
                newContact.contactPoint = { {true, contactPoint},
                                    {true, contactPoint - newContact.collisionNormal * newContact.penetrationDepth} };
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
                Vector3 vertexOne = GetBoxContactVertexLocal({ scl1.x,scl1.y,scl1.z }, axes1, newContact.collisionNormal, Greater);
                Vector3 vertexTwo = GetBoxContactVertexLocal({ scl2.x,scl2.y,scl2.z }, axes2, newContact.collisionNormal, Less);

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
                edge1 = (vertexOne[testAxis1] < 0) ? axes1[testAxis1] : axes1[testAxis1] * -1.f;
                edge2 = (vertexTwo[testAxis2] < 0) ? axes2[testAxis2] : axes2[testAxis2] * -1.f;

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
            collisionData.bodies[0] = isDynamic1 ? obj1.GetRigidBody() : nullptr;
            collisionData.bodies[1] = isDynamic2 ? obj2.GetRigidBody() : nullptr;
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

            bool isDynamic1 = obj1.IsDynamic();
            bool isDynamic2 = obj2.IsDynamic();

            Vec3 extents1 = std::get<Vec3>(box1->GetScale());
            Vec3 extents2 = std::get<Vec3>(box2->GetScale());

            float radius1 = std::max({ extents1.x, extents1.y, extents1.z });
            float radius2 = std::max({ extents2.x, extents2.y, extents2.z });

            Vector3 position1 = isDynamic1 ? obj1.GetRigidBody()->GetPosition() : Vector3(obj1.GetModelRTMatrix()[3].x, obj1.GetModelRTMatrix()[3].y, obj1.GetModelRTMatrix()[3].z);
            Vector3 position2 = isDynamic2 ? obj2.GetRigidBody()->GetPosition() : Vector3(obj2.GetModelRTMatrix()[3].x, obj2.GetModelRTMatrix()[3].y, obj2.GetModelRTMatrix()[3].z);

            Vector3 distanceVec = position2 - position1;
            if (distanceVec.LengthSquared() > (radius1 + radius2) * (radius1 + radius2)) {
                return; // No collision
            }

            // Axes to test for potential separating planes
            std::vector<Vector3> axes;

            // Axes of the first box
            for (int i = 0; i < 3; ++i) {
                axes.push_back(isDynamic1 ? obj1.GetRigidBody()->GetAxis(i) : Vector3(obj1.GetModelRTMatrix()[i].x, obj1.GetModelRTMatrix()[i].y, obj1.GetModelRTMatrix()[i].z));
            }

            // Axes of the second box
            for (int i = 0; i < 3; ++i) {
                axes.push_back(isDynamic2 ? obj2.GetRigidBody()->GetAxis(i) : Vector3(obj1.GetModelRTMatrix()[i].x, obj1.GetModelRTMatrix()[i].y, obj1.GetModelRTMatrix()[i].z));
            }

            // Edge-edge axes (cross products)
            for (int i = 0; i < 3; ++i) {
                for (int j = 0; j < 3; ++j) {
                    Vector3 cross = axes[i].Cross(axes[3 + j]);
                    if (cross.LengthSquared() > 0.001f) {
                        axes.push_back(cross.Normalize());
                    }
                }
            }

            float minPenetration = FLT_MAX;
            int minAxisIdx = 0;

            // SAT (Separating Axis Theorem) Test
            for (int i = 0; i < axes.size(); ++i) {
                float penetration = CalcPenetration(box1, box2, obj1.GetRigidBody(), obj2.GetRigidBody(), axes[i]);
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
            if (collisionNormal.Dot(position2 - position1) < 0) {
                collisionNormal = -collisionNormal;
            }

            // Determine contact points and other collision properties
            CollisionData collisionData;
            collisionData.bodies[0] = isDynamic1 ? obj1.GetRigidBody() : nullptr;
            collisionData.bodies[1] = isDynamic2 ? obj2.GetRigidBody() : nullptr;
            collisionData.collisionNormal = collisionNormal;
            collisionData.penetrationDepth = minPenetration;
            collisionData.restitution = m_objectRestitution;
            collisionData.friction = m_friction;

            CalcContactPointsBoxBox(*box1, *box2, obj1, obj2, collisionData, minAxisIdx);

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
            m_iterationLimit(30), m_penetrationTolerance(0.0005f), m_closingSpeedTolerance(0.005f) {}

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
        void ResolveCollision() {

        }
    };
}
