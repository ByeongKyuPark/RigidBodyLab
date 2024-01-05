#include <core/Scene.h>
#include <rendering/Camera.h>
#include <rendering/Renderer.h>
#include <physics/Collider.h>
#include <physics/RigidBody.h>
#include <core/Transform.h>
#include <utilities/ThreadPool.h>
#include <memory>//std::make_unique
#include <string>
#include <random>

#include <math/Matrix3.h>//temp
#include <math/Vector3.h>//temp

using namespace Physics;

Core::Scene::Scene() 
    : m_ambientLightIntensity{0.8f,0.8f,0.8f,1.f}, m_ambientAlbedo{ 1.f, 1.f, 1.f, 1.0f }, m_numLights{ 4 }, m_orbitalLights(Renderer::NUM_MAX_LIGHTS),
	m_diffuseAlbedo{ 0.9f, 0.9f, 0.9f, 1.0f }, m_specularAlbedo{ 1.f, 1.f, 1.f, 1.0f },
	m_specularPower{ 12 }, m_collisionManager{}, m_mirror{ nullptr }, m_sphere{ nullptr }
{
    SetUpScene();
    SetUpProjectiles();
    SetUpOrbitalLights();
}

void Core::Scene::SetLightColor(const Vec4& lightColor, int lightIdx)
{
    if (lightIdx >= m_numLights) {
        throw std::runtime_error("SetLightColor::light index out of rage");
    }
    m_orbitalLights[lightIdx].m_intensity = lightColor;
}

void Core::Scene::Update(float dt) {
    ApplyBroadPhase();

    // narrow phase collision detection and resolution
    ApplyNarrowPhaseAndResolveCollisions(dt);

    // integrate (multi-threading)
    ThreadPool& pool = ThreadPool::GetInstance();
    for (const auto& obj : m_objects) {

        Core::Object* rawObjPtr = obj.get();

        // enqueue the task
        pool.enqueue([rawObjPtr, dt]() {
            rawObjPtr->Integrate(dt);
            });
    }
}

int Core::Scene::AddLight() {
    if (m_numLights < Renderer::NUM_MAX_LIGHTS) {
        m_numLights++;
    }
    return m_numLights;
}
int Core::Scene::RemoveLight() {
    if (m_numLights > 0) {
        m_numLights--;
    }
    return m_numLights;
}

Core::Object& Core::Scene::GetObject(size_t index) {
    return *(m_objects.at(index)); // 'at' for bounds checking
}

const Core::Object& Core::Scene::GetObject(size_t index) const{
    return *(m_objects.at(index)); // 'at' for bounds checking
}

const Vec3& Core::Scene::GetLightPosition(int lightIdx) const {
    if (lightIdx >= m_numLights) {
        throw std::runtime_error("GetLightPosition::light index out of rage");
    }
    return m_orbitalLights[lightIdx].m_lightPosWF;
}

const Vec4& Core::Scene::GetLightColor(int lightIdx) const {
    if (lightIdx >= m_numLights) {
        throw std::runtime_error("GetLightColor:: light index out of range");
    }
    return m_orbitalLights[lightIdx].m_intensity;
}



/**
 * Creates and returns a pointer to a new Object with the specified parameters.
 *
 * The function utilizes a 'std::variant; for the collider configuration, allowing
 * the flexibility to specify either a radius for a sphere collider or a scale
 * for a box collider. This approach streamlines the function signature and
 * ensures type safety and clarity in specifying collider properties.
 *
 * @param name The name of the object to be created.
 * @param meshID The ID of the mesh to be used for the object.
 * @param textureID The ID of the texture to be applied to the object.
 * @param colliderType The type of collider to be attached to the object (e.g., BOX, SPHERE).
 * @param colliderConfig A 'std::variant' holding either a float for sphere collider's radius
 *                       or a Vec3 for box collider's scale, depending on 'colliderType'.
 * @param position The initial position of the object in the world (defaulted to {0.f, 0.f, 0.f}).
 * @param mass The mass of the object (defaulted to 1.0f).
 * @param orientation The initial orientation of the object (defaulted to an identity quaternion).
 * @return A pointer to the newly created Object.
 *
 * Usage examples:
 *   For a box collider - ColliderConfig colliderConfig = Vec3{1.f, 1.f, 1.f};
 *   For a sphere collider - ColliderConfig colliderConfig = 1.0f;
 */
Object* Core::Scene::CreateObject(const std::string& name, MeshID meshID, ImageID textureID, ColliderType colliderType, ColliderConfig colliderConfig, const Vector3& position, float mass, const Quaternion& orientation, ObjectType objType, bool isCollisionEnabled , bool isVisible)
{
    // Fetch the mesh and texture
    auto mesh = ResourceManager::GetInstance().GetMesh(meshID);

    std::unique_ptr<Collider> collider;
    // Determine the type of collider to create
    if (colliderType == ColliderType::BOX) {
        collider = std::make_unique<BoxCollider>(std::get<Vec3>(colliderConfig),isCollisionEnabled);
    }
    else if (colliderType == ColliderType::SPHERE) {
        collider = std::make_unique<SphereCollider>(std::get<float>(colliderConfig),isCollisionEnabled);
    }

    // Create the object with the appropriate collider
    if (mass != 0.f) {
        m_objects.emplace_back(std::make_unique<Core::Object>(
            name,
            mesh,
            textureID,
            std::move(collider),
            std::make_unique<RigidBody>(Transform{ position, orientation }, mass, colliderType), 
            objType, 
            isVisible
            ));
    }
    else {
        m_objects.emplace_back(std::make_unique<Core::Object>(
            name,
            mesh,
            textureID,
            std::move(collider),
            Transform{ position, orientation }, 
            objType,
            isVisible
        ));
    }
    
    return m_objects.back().get();
}

void Core::Scene::ShootProjectile(const Vector3& position) {
    static size_t nextProjectileIndex = 0;
    // start from the next projectile index and loop around the projectile pool
    for (size_t i = 0; i < m_projectiles.size(); ++i) {
        size_t idx = (nextProjectileIndex + i) % m_projectiles.size();
        if (!m_projectiles[idx].m_isActive) {
            m_projectiles[idx].Activate(position);
            nextProjectileIndex = (idx + 1) % m_projectiles.size(); // update the index for the next shot
            break;
        }
    }
}
void Core::Scene::ReloadProjectiles() {
    for (auto& projectile : m_projectiles) {
        projectile.Deactivate();
    }
}

void Core::Scene::SetUpScene() {
    using Physics::BoxCollider;
    using Physics::SphereCollider;
    using Physics::RigidBody;
    using Core::Transform;

    constexpr float BASE_POS_Y = 0.f;
    constexpr float BASE_SCL_Y = 5.f;//7.5
    constexpr float MIRROR_POS_Y = 8.4f;//5.4
    constexpr float MIRROR_SCL = 6.f;
    
    ResourceManager& resourceManager = ResourceManager::GetInstance();

    //default objects for the demo scene

    //Vec3 basePos{ 0, BASE_POS_Y, 0 };
    //Vec3 baseSize = Vec3(5.f, 5.f, 5.f);
    //CreateObject("plane", MeshID::SPHERE, ImageID::STONE_TEX_1, ColliderType::BOX, baseSize, { 0, BASE_POS_Y, 0 }, 0.f, Quaternion{}, ObjectType::DEFERRED_REGULAR);

    //(1) PLANE
    Vec3 basePos{ 0, BASE_POS_Y, 0 };
    Vec3 baseSize = Vec3(30.0f, BASE_SCL_Y,20.0f);
    CreateObject("plane", MeshID::CUBE, ImageID::STONE_TEX_1, ColliderType::BOX, baseSize, { 0, BASE_POS_Y, 0 }, 0.f, Quaternion{},ObjectType::NORMAL_MAPPED_PLANE);

    //(2) VASE
    constexpr float VASE_SCL = 3.f;
    CreateObject("vase", MeshID::VASE, ImageID::POTTERY_TEX_1, ColliderType::BOX, Vec3{ VASE_SCL ,VASE_SCL ,VASE_SCL }, { 0.5f, 4.5f, 0.5f }, 1.f, Quaternion{ 30.f,Vector3{1.f,0.f,0.f} });

    //(3) MIRROR
    Vec3 mirrorColliderSize = Vec3{ 7.f,7.f,0.5f };
    Transform mirrorTransform{ {0.75f, MIRROR_POS_Y, -1.5f} ,Quaternion{180,Vector3{0.f,1.f,0.f}} };
    std::unique_ptr<RigidBody> mirrorRigidBody = std::make_unique<RigidBody>(mirrorTransform);
    m_mirror=CreateObject("planar mirror", MeshID::PLANE, ImageID::MIRROR_TEX, ColliderType::BOX, mirrorColliderSize, {4.f, MIRROR_POS_Y, -4.5f}, 1.f, Quaternion{ 180.f,Vector3{0.f,1.f,0.f} },ObjectType::REFLECTIVE_FLAT);

    //(4) SPHERE
    constexpr float SPHERE_RAD = 3.5f;
    m_sphere = CreateObject("spherical mirror", MeshID::SPHERE, ImageID::SPHERE_TEX, ColliderType::SPHERE, SPHERE_RAD, { -4.5f, 7.f, -1.5f }, 1.f, Quaternion{},ObjectType::REFLECTIVE_CURVED);
}

void Core::Scene::ApplyBroadPhase()
{
    //TODO::Octree Partitioning 
    //https://www.gamedeveloper.com/programming/octree-partitioning-techniques
}

void Core::Scene::ApplyNarrowPhaseAndResolveCollisions(float dt)
{
    m_collisionManager.Reset();
    ThreadPool& pool = ThreadPool::GetInstance();
    const size_t objSize = m_objects.size();

    
    // detect collisions in parallel
	for (size_t i{}; i < objSize; ++i) {
		for (size_t j{ i + 1 }; j < objSize; ++j) {
			// enqueue collision checks
            pool.enqueue([this, i, j]() {
                m_collisionManager.CheckCollision(m_objects[i].get(), m_objects[j].get());
                });
		}
	}    

    // resolve stored collisions
    m_collisionManager.ResolveCollision(dt);
}

void Core::Scene::SetUpOrbitalLights() {
    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    //rendom offset between -3 and 3
    for (int i = 1; i < m_numLights; ++i) {
        float offsetX = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX) * 6.f - 3.f;
        float offsetY = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX) * 6.f - 3.f;
        float offsetZ = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX) * 6.f - 3.f;
        m_orbitalLights[i].m_lightOrbitOffset = Vec3(offsetX, offsetY, offsetZ);

        // random orbital radius between 0.5 and 20.0
        float rad = 0.5f + static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX) * 20.0f;
        m_orbitalLights[i].m_orbitalRad = rad;

        // random orbital speed between 0.1 and 0.8
        float speed = 0.1f + static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX) * 0.7f;
        m_orbitalLights[i].m_orbitalSpeed = speed;

        m_orbitalLights[i].m_accumulatedTime = 0.f;

        // random rotation axis and angle
        Vec3 axis = Vec3(
            static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX),
            1.f,
            //static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX),
            static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX)
        );
        axis = Normalize(axis); 
        float angle = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX) * 360.f; // in degrees
        m_orbitalLights[i].m_rotationAxis = axis;
        m_orbitalLights[i].m_rotationAngle = angle;


        // random light intensity for each color channel
        float intensityR = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX) *0.75f;
        float intensityG = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX) * 0.75f;
        float intensityB = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX) * 0.75f;
        m_orbitalLights[i].m_intensity = Vec4(intensityR, intensityG, intensityB, 1.f);
    }
}




//Integrated the projectiles directly into the m_objects vector within the Scene class,
//so as not to alter the whole rendering process. 
//the key is to manage these projectile objects effectively within the existing structure just for the demo.
void Core::Scene::SetUpProjectiles() {
    static constexpr float PROJECTILE_SCL = 1.f;
    m_projectiles.clear();

    std::random_device rd;
    std::mt19937 gen(rd());

    // Adjust the range to exclude the Plane mesh
    std::uniform_int_distribution<> meshDist(0, static_cast<int>(MeshID::NUM_MESHES) - 2);
    std::uniform_int_distribution<> imageDist(0, static_cast<int>(ImageID::NUM_IMAGES) - 3);//removed the mirror textures (the last 2) 

    for (int i{}; i < NUM_PROJECTILES; ++i) {
        MeshID randomMeshID = static_cast<MeshID>(meshDist(gen));

        // If the mesh is Plane or greater, increment the ID to skip Plane
        if (randomMeshID >= MeshID::PLANE) {
            randomMeshID = static_cast<MeshID>(static_cast<int>(randomMeshID) + 1);
        }

        ImageID randomImageID = static_cast<ImageID>(imageDist(gen));

        ColliderType colliderType;
        ColliderConfig colliderConfig;

        // Sphere collider for Sphere and Dodecahedron meshes
        if (randomMeshID == MeshID::SPHERE || randomMeshID == MeshID::DODECAHEDRON) {
            colliderType = ColliderType::SPHERE;
            colliderConfig = PROJECTILE_SCL; // sphere radius
        }
        else {
            colliderType = ColliderType::BOX;
            colliderConfig = Vec3{ PROJECTILE_SCL, PROJECTILE_SCL, PROJECTILE_SCL }; // box scale
        }

        Core::Object* projectile = CreateObject(
            "projectile" + std::to_string(i),
            randomMeshID,
            randomImageID,
            colliderType,
            colliderConfig,
            { mainCam.GetPos().x, mainCam.GetPos().y, mainCam.GetPos().z },
            0.5f, // mass
            Quaternion{},
            Core::ObjectType::DEFERRED_REGULAR,
            false, // turn off collision
            false // not visible by default
        );
        m_projectiles.emplace_back(projectile);
    }
}

void Core::Scene::SetLightPosition(const Vector3& lightPos, int lightIdx)
{
    if (lightIdx >= m_numLights) {
        throw std::runtime_error("SetLightPosition::light index out of rage");
    }
    m_orbitalLights[lightIdx].m_lightPosWF = lightPos;
}