#pragma once
#include <math/Vector3.h>
#include <physics/RigidBody.h>
#include <memory>//weak_ptr

namespace Physics{
    struct CollisionData {
        std::weak_ptr<RigidBody> bodies[2];
        Math::Vector3 collisionNormal; //dir : body0 <--- body1
        float penetrationDepth;
        float restitution;
        float friction;
        float accumulatedNormalImpulse; //perpendicular to the collision surface, (frictions are parallel)
        Math::Vector3 contactPoint;

        CollisionData() :collisionNormal{}, penetrationDepth{}, restitution{}, friction{}, accumulatedNormalImpulse{}, contactPoint{} {
        }
    };
}