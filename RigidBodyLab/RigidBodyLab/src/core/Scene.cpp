#include <rendering/Scene.h>
#include <rendering/Camera.h>
#include <physics/Collider.h>
#include <physics/RigidBody.h>
#include <memory>//std::make_unique

#include <math/Matrix3.h>//temp
#include <math/Vector3.h>//temp

using namespace Rendering;
using namespace Physics;

Scene::Scene() 
    : m_I{ 0.6f, 0.6f, 0.6f, 1.0f }, m_ambientAlbedo{ 0.6f, 0.6f, 0.6f, 1.0f },
m_diffuseAlbedo{ 0.6f, 0.6f, 0.6f, 1.0f }, m_specularAlbedo{ 1.0f, 1.0f, 1.0f, 1.0f },
m_specularPower{ 10 }, m_mirrorTranslate{}, m_mirrorRotationAxis{ BASIS[1] }, m_mirrorRotationAngle{}, m_spherePos{}, m_lightPosVF{ Vec3{}, }, m_lightPosWF{ Vec3{}, }, m_collisionManager{}
{
    SetUpScene();
}

void Scene::Update(float dt) {
    ApplyBroadPhase();

    // Apply narrow phase collision detection and resolution
    ApplyNarrowPhaseAndResolveCollisions(dt);

    for (auto& obj : m_objects) {
        obj->Integrate(dt);
    }
}

Core::Object& Scene::GetObject(size_t index) {
    return *(m_objects.at(index)); // 'at' for bounds checking
}

const Core::Object& Rendering::Scene::GetObject(size_t index) const{
    return *(m_objects.at(index)); // 'at' for bounds checking
}

void Scene::SetUpScene() {
    using Physics::BoxCollider;
    using Physics::SphereCollider;
    using Physics::RigidBody;

    //constexpr float BASE_POS_Y = -4.5f;
    constexpr float BASE_POS_Y = 0.f;
    constexpr float BASE_SCL_Y = 1.f;
    //constexpr float BASE_SCL_Y = 7.5f;
    constexpr float MIRROR_POS_Y = 2.4f;
    constexpr float MIRROR_SCL = 6.f;
    
    ResourceManager& resourceManager = ResourceManager::GetInstance();

    //Dependency Injection pattern

    float mass = 1.f;
    Math::Matrix3 inertiaTensor;
    inertiaTensor.SetDiagonal(1.f * CUBE_INERTIA_FACTOR);

    //(1) PLANE
    //Vec3 baseSize = Vec3(11.0f, BASE_SCL_Y, 7.0f);
    //Vec3 cubeColliderSize = baseSize; // Set appropriate dimensions for the collider
    //std::unique_ptr<RigidBody> planeRigidBody= std::make_unique<RigidBody>();
    //std::unique_ptr<BoxCollider>planeCollider = std::make_unique<BoxCollider>(cubeColliderSize);
    //auto& cubeMesh = resourceManager.GetMesh(MeshID::CUBE);
    //m_objects.emplace_back(std::make_unique<Core::Object>(cubeMesh, ImageID::STONE_TEX, Translate(0, BASE_POS_Y, 0) * Scale(baseSize), planeRigidBody.release(), planeCollider.release()));
    
    //(1) PLANE

    Vec3 basePos{ 0, BASE_POS_Y, 0 };
    //Vec3 baseSize = Vec3(11.0f, BASE_SCL_Y, 7.0f);
    Vec3 baseSize = Vec3(1.0f, 1.f, 1.0f);
    Vec3 cubeColliderSize = baseSize; // Set appropriate dimensions for the collider
    //std::unique_ptr<RigidBody> planeRigidBody = std::make_unique<RigidBody>();
    //planeRigidBody->SetPosition(Math::Vector3(basePos.x, basePos.y, basePos.z));  // Set the initial position
    //planeRigidBody->SetMass(mass);
    ////angle
    //{
    //    float angleDegrees = 45.0f; // Angle in degrees
    //    Math::Vector3 axis(0, 1, 0); // Y-axis for example

    //    // Create a quaternion representing the rotation
    //    Math::Quaternion rotation(angleDegrees, axis);

    //    // Set this rotation to the rigid body
    //    planeRigidBody->SetOrientation(rotation);
    //}
    //planeRigidBody->SetInertiaTensor(inertiaTensor);

    Mat4 planeModelToWorldMat = Translate(0, BASE_POS_Y, 0);// * Scale(baseSize);
    std::unique_ptr<BoxCollider>planeCollider = std::make_unique<BoxCollider>(cubeColliderSize);
    planeCollider->SetScale(cubeColliderSize);
    auto& cubeMesh = resourceManager.GetMesh(MeshID::CUBE);
    m_objects.emplace_back(std::make_unique<Core::Object>(cubeMesh, ImageID::STONE_TEX,std::move(planeCollider), planeModelToWorldMat));
    //m_objects.emplace_back(std::make_unique<Core::Object>(cubeMesh, ImageID::STONE_TEX, std::move(planeCollider), std::move(planeRigidBody)));


    //(2) VASE
    //Vec3 vasePos{ 0.0f, 0.995f, 0.0f };
    Vec3 vasePos{ 0.0f, 1.2f, 0.0f };
    //Vec3 vasePos{ 1.0f, -0.645f, 2.0f };
    Vec3 vaseColliderSize = {1.f,1.f,1.f};
    std::unique_ptr<RigidBody> vaseRigidBody = std::make_unique<RigidBody>();
    vaseRigidBody->SetPosition(Math::Vector3(vasePos.x, vasePos.y, vasePos.z));  // Set the initial position
    vaseRigidBody->SetMass(mass);
    vaseRigidBody->SetInertiaTensor(inertiaTensor);

    std::unique_ptr<BoxCollider>vaseCollider = std::make_unique<BoxCollider>(vaseColliderSize);
    //auto& vaseMesh = resourceManager.GetMesh(MeshID::VASE);
    m_objects.emplace_back(std::make_unique<Core::Object>(cubeMesh, ImageID::POTTERY_TEX, std::move(vaseCollider), std::move(vaseRigidBody)));

    ////(3) MIRROR
    //m_mirrorTranslate = Vec3(1.0f, MIRROR_POS_Y, -1.5f);
    //m_mirrorRotationAxis = BASIS[Y];
    //m_mirrorRotationAngle = -EIGHTH_PI;
    //Vec3 mirrorColliderSize = Vec3{ 10.f,10.f,10.f };//temp
    //std::unique_ptr<RigidBody> mirrorRigidBody = std::make_unique<RigidBody>();
    //std::unique_ptr<BoxCollider> mirrorCollider = std::make_unique<BoxCollider>(mirrorColliderSize);
    //auto& planeMesh = resourceManager.GetMesh(MeshID::PLANE);
    //m_objects.emplace_back(std::make_unique<Core::Object>(planeMesh, ImageID::MIRROR_TEX, Translate(m_mirrorTranslate) * Scale(MIRROR_SCL, MIRROR_SCL, MIRROR_SCL) * Rotate(PI + m_mirrorRotationAngle, m_mirrorRotationAxis),std::move(mirrorRigidBody), std::move(mirrorCollider)));

    //// Setup the base of the mirror
    //constexpr float MIRROR_FRAME_OFFSET = 0.45f;
    //Vec3 mirrorPartColliderSize = Vec3{ 5.f,5.f,5.f };//temp
    //std::unique_ptr<RigidBody> mirrorPart1RigidBody = std::make_unique<RigidBody>();
    //std::unique_ptr<BoxCollider> mirrorPart1Collider = std::make_unique<BoxCollider>(mirrorPartColliderSize);
    //m_objects.emplace_back(std::make_unique<Core::Object>(cubeMesh, ImageID::WOOD_TEX, Translate(m_mirrorTranslate + Vec3(0, 0, -0.03f)) * Rotate(m_mirrorRotationAngle, m_mirrorRotationAxis) * Scale(MIRROR_SCL + MIRROR_FRAME_OFFSET, MIRROR_SCL + MIRROR_FRAME_OFFSET, 0.05f),std::move(mirrorPart1RigidBody),std::move(mirrorPart1Collider)));

    //std::unique_ptr<RigidBody> mirrorPart2RigidBody = std::make_unique<RigidBody>();
    //std::unique_ptr<BoxCollider> mirrorPart2Collider = std::make_unique<BoxCollider>(mirrorPartColliderSize);
    //m_objects.emplace_back(std::make_unique<Core::Object>(cubeMesh, ImageID::WOOD_TEX, Translate(m_mirrorTranslate + Vec3(0, -3.1, -0.6f)) * Rotate(m_mirrorRotationAngle, m_mirrorRotationAxis) * Scale(3.0f, 0.1f, 1.0f), std::move(mirrorPart2RigidBody), std::move(mirrorPart2Collider)));

    //std::unique_ptr<RigidBody> mirrorPart3RigidBody = std::make_unique<RigidBody>();
    //std::unique_ptr<BoxCollider> mirrorPart3Collider = std::make_unique<BoxCollider>(mirrorPartColliderSize);
    //m_objects.emplace_back(std::make_unique<Core::Object>(cubeMesh, ImageID::WOOD_TEX, Translate(m_mirrorTranslate + Vec3(0, -2.1, -0.53f)) * Rotate(m_mirrorRotationAngle, m_mirrorRotationAxis) * Rotate(TWO_PI / 3, BASIS[X]) * Scale(0.5f, 0.1f, 1.7f), std::move(mirrorPart3RigidBody), std::move(mirrorPart3Collider)));

    ////(4) SPHERE
    //constexpr float SPHERE_RAD = 4.5f;
    //// Setup the sphere
    //std::unique_ptr<RigidBody> sphereRigidBody = std::make_unique<RigidBody>();
    //std::unique_ptr<SphereCollider> sphereCollider = std::make_unique<SphereCollider>(SPHERE_RAD);
    //m_spherePos = Vec3(-4.5f, BASE_POS_Y + BASE_SCL_Y / 2.f + SPHERE_RAD / 2.f, -1.5f);
    //auto& sphereMesh = resourceManager.GetMesh(MeshID::SPHERE);
    //m_objects.emplace_back(std::make_unique<Core::Object>(sphereMesh, ImageID::SPHERE_TEX, Translate(m_spherePos) * Scale(SPHERE_RAD, SPHERE_RAD, SPHERE_RAD), std::move(sphereRigidBody), std::move(sphereCollider)));

    SetUpLight(baseSize.x);
}

void Rendering::Scene::ApplyBroadPhase()
{
}

void Rendering::Scene::ApplyNarrowPhaseAndResolveCollisions(float dt)
{
    m_collisionManager.Clear();
    const size_t objSize = m_objects.size();
    // Detect and store collisions
    for (size_t i{}; i < objSize; ++i) {
        for (size_t j{ i + 1 }; j < objSize; ++j) {
            m_collisionManager.CheckCollision(*m_objects[i], *m_objects[j]);
        }
    }

    // Resolve stored collisions
    m_collisionManager.ResolveCollision(dt);
}



void Rendering::Scene::SetUpLight(float height){
    m_lightPosWF[0] = Vec3(0, height, 0);
}
