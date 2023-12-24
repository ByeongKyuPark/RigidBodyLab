#include <core/Scene.h>
#include <rendering/Camera.h>
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
    : m_I{ 0.6f, 0.6f, 0.6f, 1.0f }, m_ambientAlbedo{ 0.6f, 0.6f, 0.6f, 1.0f },
m_diffuseAlbedo{ 0.6f, 0.6f, 0.6f, 1.0f }, m_specularAlbedo{ 1.0f, 1.0f, 1.0f, 1.0f },
m_specularPower{ 10 }, m_lightPosVF{ Vec3{}, }, m_lightPosWF{ Vec3{}, }, m_collisionManager{}, m_mirror{ nullptr }, m_sphere{nullptr}
{
    SetUpScene();
    SetUpProjectiles();
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

Core::Object& Core::Scene::GetObject(size_t index) {
    return *(m_objects.at(index)); // 'at' for bounds checking
}

const Core::Object& Core::Scene::GetObject(size_t index) const{
    return *(m_objects.at(index)); // 'at' for bounds checking
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
Object* Core::Scene::CreateObject(const std::string& name, MeshID meshID, ImageID textureID, ColliderType colliderType, ColliderConfig colliderConfig, const Vector3& position, float mass, const Quaternion& orientation, ObjectType objType, bool isCollisionEnabled )
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
            objType
            ));
    }
    else {
        m_objects.emplace_back(std::make_unique<Core::Object>(
            name,
            mesh,
            textureID,
            std::move(collider),
            Transform{ position, orientation }, 
            objType
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
    constexpr float BASE_SCL_Y = 2.f;//7.5
    constexpr float MIRROR_POS_Y = 8.4f;//5.4
    constexpr float MIRROR_SCL = 6.f;
    
    ResourceManager& resourceManager = ResourceManager::GetInstance();

    //default objects for the demo scene
    
    //(1) PLANE
    Vec3 basePos{ 0, BASE_POS_Y, 0 };
    Vec3 baseSize = Vec3(30.0f, BASE_SCL_Y,20.0f);
    CreateObject("plane", MeshID::CUBE, ImageID::STONE_TEX_1, ColliderType::BOX, baseSize, { 0, BASE_POS_Y, 0 }, 0.f, Quaternion{},ObjectType::MAPPABLE_PLANE);

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

    SetUpLight(baseSize.x);
}

void Core::Scene::ApplyBroadPhase()
{
    //TODO::Octree Partitioning 
    //https://www.gamedeveloper.com/programming/octree-partitioning-techniques
}

void Core::Scene::ApplyNarrowPhaseAndResolveCollisions(float dt)
{
    m_collisionManager.Reset();
    const size_t objSize = m_objects.size();
    // Detect and store collisions
    for (size_t i{}; i < objSize; ++i) {
        for (size_t j{ i + 1 }; j < objSize; ++j) {
            m_collisionManager.CheckCollision(m_objects[i].get(), m_objects[j].get());
        }
    }

    // Resolve stored collisions
    m_collisionManager.ResolveCollision(dt);
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
            Core::ObjectType::REGULAR,
            false // turn off collision
        );
        m_projectiles.emplace_back(projectile);
    }
}


void Core::Scene::SetUpLight(float height){
    m_lightPosWF[0] = Vec3(0, height, 0);
}
