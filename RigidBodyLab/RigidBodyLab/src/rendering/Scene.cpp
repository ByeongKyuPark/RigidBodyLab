#include <rendering/Scene.h>
#include <rendering/Camera.h>

using namespace Rendering;

Scene::Scene() 
    : m_I{ 0.6f, 0.6f, 0.6f, 1.0f }, m_ambientAlbedo{ 0.6f, 0.6f, 0.6f, 1.0f },
m_diffuseAlbedo{ 0.6f, 0.6f, 0.6f, 1.0f }, m_specularAlbedo{ 1.0f, 1.0f, 1.0f, 1.0f },
m_specularPower{ 10 }, m_mirrorTranslate{}, m_mirrorRotationAxis{ BASIS[1] }, m_mirrorRotationAngle{}, m_spherePos{}, m_lightPosVF{ Vec3{}, }, m_lightPosWF{ Vec3{}, }
{
    SetUpScene();
}

Object& Rendering::Scene::GetObject(size_t index) {
    return m_objects.at(index); // 'at' for bounds checking
}

const Object& Rendering::Scene::GetObject(size_t index) const{
    return m_objects.at(index); // 'at' for bounds checking
}

void Scene::SetUpScene() {
    constexpr float BASE_POS_Y = -4.5f;
    constexpr float BASE_SCL_Y = 7.5f;
    constexpr float MIRROR_POS_Y = 2.4f;
    constexpr float MIRROR_SCL = 6.f;
    
    ResourceManager& resourceManager = ResourceManager::GetInstance();
    //Dependency Injection pattern
    Vec3 baseSize = Vec3(11.0f, BASE_SCL_Y, 7.0f);
    auto& cubeMesh = resourceManager.GetMesh(MeshID::CUBE);
    m_objects.emplace_back(cubeMesh, ImageID::STONE_TEX, Translate(0, BASE_POS_Y, 0) * Scale(baseSize));

    auto& vaseMesh = resourceManager.GetMesh(MeshID::VASE);
    m_objects.emplace_back(vaseMesh, ImageID::POTTERY_TEX, Translate(1.0f, -0.65f, 2.0f));

    m_mirrorTranslate = Vec3(1.0f, MIRROR_POS_Y, -1.5f);
    m_mirrorRotationAxis = BASIS[Y];
    m_mirrorRotationAngle = -EIGHTH_PI;
    auto& planeMesh = resourceManager.GetMesh(MeshID::PLANE);
    m_objects.emplace_back(planeMesh, ImageID::MIRROR_TEX, Translate(m_mirrorTranslate) * Scale(MIRROR_SCL, MIRROR_SCL, MIRROR_SCL) * Rotate(PI + m_mirrorRotationAngle, m_mirrorRotationAxis));

    // Setup the base of the mirror
    constexpr float MIRROR_FRAME_OFFSET = 0.45f;
    m_objects.emplace_back(cubeMesh, ImageID::WOOD_TEX, Translate(m_mirrorTranslate + Vec3(0, 0, -0.03f)) * Rotate(m_mirrorRotationAngle, m_mirrorRotationAxis) * Scale(MIRROR_SCL + MIRROR_FRAME_OFFSET, MIRROR_SCL + MIRROR_FRAME_OFFSET, 0.05f));
    m_objects.emplace_back(cubeMesh, ImageID::WOOD_TEX, Translate(m_mirrorTranslate + Vec3(0, -3.1, -0.6f)) * Rotate(m_mirrorRotationAngle, m_mirrorRotationAxis) * Scale(3.0f, 0.1f, 1.0f));
    m_objects.emplace_back(cubeMesh, ImageID::WOOD_TEX, Translate(m_mirrorTranslate + Vec3(0, -2.1, -0.53f)) * Rotate(m_mirrorRotationAngle, m_mirrorRotationAxis) * Rotate(TWO_PI / 3, BASIS[X]) * Scale(0.5f, 0.1f, 1.7f));

    // Setup the sphere
    constexpr float SPHERE_RAD = 4.5f;
    m_spherePos = Vec3(-4.5f, BASE_POS_Y + BASE_SCL_Y / 2.f + SPHERE_RAD / 2.f, -1.5f);
    auto& sphereMesh = resourceManager.GetMesh(MeshID::SPHERE);
    m_objects.emplace_back(sphereMesh, ImageID::SPHERE_TEX, Translate(m_spherePos) * Scale(SPHERE_RAD, SPHERE_RAD, SPHERE_RAD));

    SetUpLight(baseSize.x);
}

void Rendering::Scene::SetUpLight(float height){
    m_lightPosWF[0] = Vec3(0, height, 0);
}
