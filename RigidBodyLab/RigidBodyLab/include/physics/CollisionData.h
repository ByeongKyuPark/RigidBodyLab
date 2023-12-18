#pragma once
#include <math/Vector3.h>
#include <physics/RigidBody.h>
#include <memory>//weak_ptr

namespace Physics{
    struct ContactPoint {
        // If bool is false, either p1 or p2 are invalid and the contact points has not been found.
        // p1 and p2 are the contacts point on the object.
        std::pair<bool, Vector3> p1, p2;
    };
    struct CollisionData {
        RigidBody* bodies[2];
        Math::Vector3 collisionNormal; //dir : body0 <--- body1
        float penetrationDepth;
        float restitution;
        float friction;
        float accumulatedNormalImpulse; //perpendicular to the collision surface, (frictions are parallel)
        ContactPoint contactPoint;

        CollisionData() :collisionNormal{}, penetrationDepth{}, restitution{}, friction{}, accumulatedNormalImpulse{}, contactPoint{} {
        }
    };
}