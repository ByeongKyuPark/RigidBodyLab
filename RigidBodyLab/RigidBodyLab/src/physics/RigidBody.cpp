#include <physics/RigidBody.h>
#include <cmath>
#include <iostream>

using namespace Physics;

void RigidBody::Integrate(float duration)
{
    if (massInverse == 0.0f) {
        return;
    }
    
    //0. apply gravitational force
    static Vector3 gravity(0, -0.0981f, 0);
    force += gravity * (1.0f / massInverse); // Gravity force

    //1. linear Velocity
    Vector3 linearAcceleration = force * massInverse;
    velocity += linearAcceleration * duration;
    velocity *= powf(linearDamping, duration);

    //2. angular Velocity
    Vector3 angularAcceleration = inverseInertiaTensorWorld * torque;
    angularVelocity += angularAcceleration * duration;
    angularVelocity *= powf(angularDamping, duration);

    //3.Pos, Orientation
    transform.position += velocity * duration;
    //std::cout << position << "\n";
    transform.orientation += transform.orientation.RotateByVector(angularVelocity, duration / 2.0f);
    transform.orientation.Normalize();

    //5.update accordingly
    transform.Update();
    TransformInertiaTensor();

    force.Clear();
    torque.Clear();
}

void RigidBody::AddForce(const Vector3& _force){
    force += _force;
}

void RigidBody::AddForceAt(const Vector3& _force, const Vector3& point)
{
    force += _force;
    Vector3 pointFromCenter = point - transform.position;
    torque += pointFromCenter.Cross(_force);
}

Vector3 RigidBody::GetAxis(int index) const
{
    return transform.GetAxis(index);
}

void RigidBody::RotateByQuat(const Quaternion& quat)
{
    Quaternion newOrientation = GetOrientation() * quat;
    newOrientation.Normalize();
    SetOrientation(newOrientation);
}


void RigidBody::TransformInertiaTensor()
{
    Matrix3 rotationMatrix=transform.localToWorld.Extract3x3Matrix();
    inverseInertiaTensorWorld = (rotationMatrix * inverseInertiaTensor) * rotationMatrix.Transpose();
}

void RigidBody::SetMass(float value)
{
    massInverse = 1.0f / value;
}

void RigidBody::SetInverseMass(float value)
{
    massInverse = value;
}

void RigidBody::SetInertiaTensor(const Matrix3& mat)
{
    inverseInertiaTensor = mat.Inverse();
    TransformInertiaTensor();
}

void RigidBody::SetInverseInertiaTensor(const Matrix3& mat)
{
    inverseInertiaTensor = mat;
    TransformInertiaTensor();
}

void RigidBody::SetPosition(const Vector3& vec)
{
    transform.position = vec;
    transform.Update();
}

void RigidBody::SetPosition(float x, float y, float z)
{
    transform.position.x = x;
    transform.position.y = y;
    transform.position.z = z;
    transform.Update();
}

void RigidBody::SetOrientation(const Quaternion& quat)
{
    transform.orientation = quat;
    transform.Update();
    TransformInertiaTensor();
}

void RigidBody::SetLinearVelocity(const Vector3& vec)
{
    velocity = vec;
}

void RigidBody::SetLinearVelocity(float x, float y, float z)
{
    velocity.x = x;
    velocity.y = y;
    velocity.z = z;
}

void RigidBody::SetAngularVelocity(const Vector3& vec){
    angularVelocity = transform.localToWorld.Extract3x3Matrix() * vec;
}

void RigidBody::SetAngularVelocity(float x, float y, float z){
    angularVelocity = transform.localToWorld.Extract3x3Matrix() * Vector3(x, y, z);
}

void RigidBody::SetLinearAcceleration(const Vector3& vec){
    linearAcceleration = vec;
}

void RigidBody::SetLinearAcceleration(float x, float y, float z){
    linearAcceleration.x = x;
    linearAcceleration.y = y;
    linearAcceleration.z = z;
}

void RigidBody::SetLinearDamping(float value){
    linearDamping = value;
}

float RigidBody::GetMass() const{
    return 1.0f / massInverse;
}

float RigidBody::GetInverseMass() const{
    return massInverse;
}

Matrix3 RigidBody::GetInverseInertiaTensor() const{
    return inverseInertiaTensor;
}

Matrix3 RigidBody::GetInverseInertiaTensorWorld() const{
    return inverseInertiaTensorWorld;
}

Vector3 RigidBody::GetPosition() const{
    return transform.position;
}

Vector3 RigidBody::GetLinearVelocity() const{
    return velocity;
}

Vector3 RigidBody::GetAngularVelocity() const{
    return transform.localToWorld.Extract3x3Matrix().Transpose() * angularVelocity;
}

Vector3 RigidBody::GetAcceleration() const{
    return linearAcceleration;
}

float RigidBody::GetLinearDamping() const{
    return linearDamping;
}

Physics::Matrix4 Physics::RigidBody::GetLocalToWorldMatrix() const{
    return transform.localToWorld;
}

glm::mat4 RigidBody::GetLocalToWorldMatrixGLM() const{
    return transform.localToWorld.ConvertToGLM();
}
