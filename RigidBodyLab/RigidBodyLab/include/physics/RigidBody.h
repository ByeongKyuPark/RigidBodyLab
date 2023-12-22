#pragma once
#include <math/Matrix3.h>
#include <core/Transform.h>
#include <physics/Collider.h>

namespace Physics
{
    using Math::Matrix3;
    using Math::Matrix4;
    using Math::Vector3;
    using Math::Quaternion;

    class RigidBody
    {
    protected:        
        Core::Transform transform;
        Matrix3 inverseInertiaTensor; //local
        Matrix3 inverseInertiaTensorWorld; //world
        // Represents the transformation matrix without scale. Avoids skewing and simulates the rigid body as a point mass for simplified physics calculations.

        Vector3 velocity;
        Vector3 angularVelocity;
        Vector3 linearAcceleration;
        Vector3 force;
        Vector3 torque;

        float massInverse;
        float linearDamping;
        float angularDamping;

        static constexpr float SPHERE_INERTIA_FACTOR = 0.4f;
        static constexpr float CUBE_INERTIA_FACTOR = 1 / 6.0f;
    public:
        RigidBody(Core::Transform& _transform, float _mass = 1.f, ColliderType colliderType=ColliderType::BOX) : transform{ _transform }, massInverse{ 1.f/_mass }, linearDamping(0.9f), angularDamping(0.75f)
        {
            Math::Matrix3 inertiaTensor;
            float diagonal = _mass *  (colliderType == ColliderType::SPHERE) ?  SPHERE_INERTIA_FACTOR : CUBE_INERTIA_FACTOR;
            inertiaTensor.SetDiagonal(diagonal);
            SetInertiaTensor(inertiaTensor);
        }

        void Integrate(float duration);
        void AddForceAt(const Vector3& force, const Vector3& point);
        void AddForce(const Vector3& force);
        Vector3 GetAxis(int index) const;
        void RotateByQuat(const Quaternion&);
        bool IsFixed() {return massInverse == 0.0f ? true : false;}

    private:    
        //no scale, simulates the rigid body as a point mass for simplified physics calculations.
        void TransformInertiaTensor();

    public:
        void SetMass(float value);
        void SetInverseMass(float value);

        void SetInertiaTensor(const Matrix3& mat);
        void SetInverseInertiaTensor(const Matrix3& mat);

        void SetPosition(const Vector3& vec);
        void SetPosition(float x, float y, float z);

        void SetOrientation(const Quaternion&);
        
        void SetLinearVelocity(const Vector3& vec);
        void SetLinearVelocity(float x, float y, float z);

        void SetAngularVelocity(const Vector3& vec);
        void SetAngularVelocity(float x, float y, float z);

        void SetLinearAcceleration(const Vector3& vec);
        void SetLinearAcceleration(float x, float y, float z);

        void SetLinearDamping(float value);

        float GetMass() const;
        float GetInverseMass() const;
        Matrix3 GetInverseInertiaTensor() const;
        Matrix3 GetInverseInertiaTensorWorld() const;
        Vector3 GetPosition() const;
        Quaternion GetOrientation() const { return transform.m_orientation; }
        Vector3 GetLinearVelocity() const;
        Vector3 GetAngularVelocity() const;
        Vector3 GetAcceleration() const;
        float GetLinearDamping() const;
        Matrix4 GetLocalToWorldMatrix() const;
        glm::mat4 GetLocalToWorldMatrixGLM() const;
    }; 
}