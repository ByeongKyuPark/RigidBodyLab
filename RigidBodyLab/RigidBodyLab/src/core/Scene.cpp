#include <core/Scene.h>
#include <rendering/Camera.h>
#include <rendering/Renderer.h>
#include <physics/Collider.h>
#include <physics/RigidBody.h>
#include <core/Transform.h>
//#include <utilities/ThreadPool.h>
#include <memory>//std::make_unique
#include <string>
#include <random>

#include <math/Matrix3.h>//temp
#include <math/Vector3.h>//temp

using namespace Physics;

Core::Scene::Scene() 
    : m_ambientLightIntensity{0.3f,0.3f,0.3f,1.f}, m_ambientAlbedo{ 1.f, 1.f, 1.f, 1.0f }, m_numLights{ 1 }, m_orbitalLights(Renderer::NUM_MAX_LIGHTS),
	m_diffuseAlbedo{ 0.9f, 0.9f, 0.9f, 1.0f }, m_specularAlbedo{ 1.f, 1.f, 1.f, 1.0f },
	m_specularPower{ 12 }, m_collisionManager{}, m_mirror{ nullptr }, m_idol{ nullptr }
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
    if (m_idol!=nullptr) {//if the idol alive
        ShrinkPlaneOverTime(dt);
    }
    
    //ApplyBroadPhase();

    // narrow phase collision detection and resolution
    ApplyNarrowPhaseAndResolveCollisions(dt);

    // integrate (multi-threading)
    //ThreadPool& pool = ThreadPool::GetInstance();
    for (const auto& obj : m_objects) {
        // enqueue the task
        //pool.enqueue([rawObjPtr, dt]() {
        if (obj.get()->IsVisible() == true) {
            obj.get()->Integrate(dt);
        }
        //});
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
    if (colliderType == ColliderType::OBB) {
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

void Scene::RemoveObjectsBelowThreshold() {
    RemoveProjectiles();
    RemoveAndNullifySpecialObjects();    
}

void Core::Scene::RemoveProjectiles(){
    // deactivate projectiles that fall below the threshold
    // first, mark projectiles for removal (as projectiles is also an object, need to delete from the projectiles first)
    for (auto& projectile : m_projectiles) {
        if (projectile.m_object->GetPosition().y < Y_THRESHOLD) {
            projectile.m_hasKnockedOff = true;
        }
    }

    // then, remove knocked off projectiles
    m_projectiles.erase(std::remove_if(m_projectiles.begin(), m_projectiles.end(),
        [](const Projectile& projectile) {
            return projectile.m_hasKnockedOff;
        }), m_projectiles.end());
}

void Core::Scene::RemoveAndNullifySpecialObjects() {
    for (auto& obj : m_objects) {
        if (obj->GetPosition().y < Y_THRESHOLD) {
            // check if the object is the sphere or mirror and nullify accordingly
            if (obj.get() == m_mirror) {
                m_mirror = nullptr; 
            }
            else if (obj.get() == m_idol) {
                m_idol = nullptr;                
            }
            else if (obj.get() == m_plane) {
                m_plane = nullptr;
            }
        }
    }

    // after handling special objects, remove objects below the threshold
    m_objects.erase(std::remove_if(m_objects.begin(), m_objects.end(),
        [](const std::unique_ptr<Core::Object>& obj) {
            return !obj || obj->GetPosition().y < Y_THRESHOLD;
        }),
        m_objects.end());
}

void Core::Scene::SetUpScene() {
    using Physics::BoxCollider;
    using Physics::SphereCollider;
    using Physics::RigidBody;
    using Core::Transform;

    constexpr float BASE_POS_Y = 0.f;
    constexpr float BASE_SCL = 70.f;
    constexpr float BASE_SCL_Y = 1.5f;//7.5
    constexpr float MIRROR_POS_Y = 10.4f;//5.4
    constexpr float MIRROR_SCL = 6.f;
    
    ResourceManager& resourceManager = ResourceManager::GetInstance();

    //default objects for the demo scene

    //Vec3 basePos{ 0, BASE_POS_Y, 0 };
    //Vec3 baseSize = Vec3(5.f, 5.f, 5.f);
    //CreateObject("plane", MeshID::SPHERE, ImageID::STONE_TEX_1, ColliderType::BOX, baseSize, { 0, BASE_POS_Y, 0 }, 0.f, Quaternion{}, ObjectType::DEFERRED_REGULAR);

    //(1) PLANE
    Vec3 basePos{ 0, BASE_POS_Y, 0 };
    Vec3 baseSize = Vec3(BASE_SCL, BASE_SCL_Y, BASE_SCL);
    m_plane=CreateObject("plane", MeshID::CUBE, ImageID::STONE_TEX_1, ColliderType::OBB, baseSize, { 0, BASE_POS_Y, 0 }, 0.f, Quaternion{},ObjectType::NORMAL_MAPPED_PLANE);

    //(2) grim reaper
    constexpr float GRIM_REAPER_SCL = 5.f;
    CreateObject("grim reaper(L) 1", MeshID::GRIM_REAPER_LEFTY, ImageID::RIPPLE, ColliderType::OBB, Vec3{ GRIM_REAPER_SCL ,GRIM_REAPER_SCL ,GRIM_REAPER_SCL }, { -10.0f, 5.0f, -10.0f }, 1.f, Quaternion{ 45.f,Vector3{0.f,1.f,0.f} });
    CreateObject("grim reaper(L) 2", MeshID::GRIM_REAPER_LEFTY, ImageID::RIPPLE, ColliderType::OBB, Vec3{ GRIM_REAPER_SCL ,GRIM_REAPER_SCL ,GRIM_REAPER_SCL }, { 10.0f, 5.0f, -10.0f }, 1.f, Quaternion{ -45.f,Vector3{0.f,1.f,0.f} });
    CreateObject("grim reaper(L) 3", MeshID::GRIM_REAPER_LEFTY, ImageID::RIPPLE, ColliderType::OBB, Vec3{ GRIM_REAPER_SCL ,GRIM_REAPER_SCL ,GRIM_REAPER_SCL }, { 10.0f, 5.0f, 10.0f }, 1.f, Quaternion{-135.f,Vector3{0.f,1.f,0.f} });
    CreateObject("grim reaper(L) 4", MeshID::GRIM_REAPER_LEFTY, ImageID::RIPPLE, ColliderType::OBB, Vec3{ GRIM_REAPER_SCL ,GRIM_REAPER_SCL ,GRIM_REAPER_SCL }, { -10.0f, 5.0f, 10.0f }, 1.f, Quaternion{ 135.f,Vector3{0.f,1.f,0.f} });
    CreateObject("grim reaper(L) 5", MeshID::GRIM_REAPER_LEFTY, ImageID::RIPPLE, ColliderType::OBB, Vec3{ GRIM_REAPER_SCL ,GRIM_REAPER_SCL ,GRIM_REAPER_SCL }, { 0.0f, 5.0f, -10.0f }, 1.f, Quaternion{ 0.f,Vector3{0.f,1.f,0.f} });
    CreateObject("grim reaper(L) 6", MeshID::GRIM_REAPER_LEFTY, ImageID::RIPPLE, ColliderType::OBB, Vec3{ GRIM_REAPER_SCL ,GRIM_REAPER_SCL ,GRIM_REAPER_SCL }, { 10.0f, 5.0f, 0.0f }, 1.f, Quaternion{ -90.f,Vector3{0.f,1.f,0.f} });
    CreateObject("grim reaper(L) 7", MeshID::GRIM_REAPER_LEFTY, ImageID::RIPPLE, ColliderType::OBB, Vec3{ GRIM_REAPER_SCL ,GRIM_REAPER_SCL ,GRIM_REAPER_SCL }, { 0.0f, 5.0f, 10.0f }, 1.f, Quaternion{ 180.f,Vector3{0.f,1.f,0.f} });
    CreateObject("grim reaper(L) 8", MeshID::GRIM_REAPER_LEFTY, ImageID::RIPPLE, ColliderType::OBB, Vec3{ GRIM_REAPER_SCL ,GRIM_REAPER_SCL ,GRIM_REAPER_SCL }, { -10.0f, 5.0f, 0.0f }, 1.f, Quaternion{ 90.f,Vector3{0.f,1.f,0.f} });

    //(3) cat
    constexpr float CAT_SCL = 1.5f;
    CreateObject("cat 1", MeshID::CAT, ImageID::POTTERY_TEX_1, ColliderType::OBB, Vec3{ CAT_SCL,CAT_SCL,CAT_SCL }, { -5.0f, 3.5f, 5.0f }, 1.f, Quaternion{ -45.f,Vector3{0.f,1.f,0.f} });
    CreateObject("cat 2", MeshID::CAT, ImageID::POTTERY_TEX_1, ColliderType::OBB, Vec3{ CAT_SCL,CAT_SCL,CAT_SCL }, { 5.0f, 3.5f, 5.0f }, 1.f, Quaternion{ 45.f,Vector3{0.f,1.f,0.f} });
    CreateObject("cat 3", MeshID::CAT, ImageID::POTTERY_TEX_1, ColliderType::OBB, Vec3{ CAT_SCL,CAT_SCL,CAT_SCL }, { -5.0f, 3.5f, -5.0f }, 1.f, Quaternion{ -135.f,Vector3{0.f,1.f,0.f} });
    CreateObject("cat 4", MeshID::CAT, ImageID::POTTERY_TEX_1, ColliderType::OBB, Vec3{ CAT_SCL,CAT_SCL,CAT_SCL }, { 5.0f, 3.5f, -5.0f }, 1.f, Quaternion{ 135.f,Vector3{0.f,1.f,0.f} });

    //(5) girl
    constexpr float CHARACTER_SCL = 3.f;
    CreateObject("character(R) 1", MeshID::GIRL_RIGHTY, ImageID::WHITE_PAPER, ColliderType::OBB, Vec3{ CHARACTER_SCL,CHARACTER_SCL,CHARACTER_SCL }, { -12.0f, 4.5f, -5.0f }, 1.f, Quaternion{ -150.f,Vector3{0.f,1.f,0.f} });
    CreateObject("character(R) 2", MeshID::GIRL_RIGHTY, ImageID::WHITE_PAPER, ColliderType::OBB, Vec3{ CHARACTER_SCL,CHARACTER_SCL,CHARACTER_SCL }, { -12.0f, 4.5f, 5.0f }, 1.f, Quaternion{ -80.f,Vector3{0.f,1.f,0.f} });
    CreateObject("character(R) 3", MeshID::GIRL_RIGHTY, ImageID::WHITE_PAPER, ColliderType::OBB, Vec3{ CHARACTER_SCL,CHARACTER_SCL,CHARACTER_SCL }, { -5.0f, 4.5f, 12.0f }, 1.f, Quaternion{ -30.f,Vector3{0.f,1.f,0.f} });
    //CreateObject("character(R) 3", MeshID::GIRL_RIGHTY, ImageID::WOOD_TEX_1, ColliderType::OBB, Vec3{ CHARACTER_SCL,CHARACTER_SCL,CHARACTER_SCL }, { 12.0f, 4.5f, -5.0f }, 1.f, Quaternion{ 60.f,Vector3{0.f,1.f,0.f} });
    //CreateObject("character(R) 4", MeshID::GIRL_RIGHTY, ImageID::WOOD_TEX_1, ColliderType::OBB, Vec3{ CHARACTER_SCL,CHARACTER_SCL,CHARACTER_SCL }, { 12.0f, 4.5f, 5.0f }, 1.f, Quaternion{ 60.f,Vector3{0.f,1.f,0.f} });

    //CreateObject("character(L) 3", MeshID::GIRL_LEFTY, ImageID::WOOD_TEX_1, ColliderType::OBB, Vec3{ CHARACTER_SCL,CHARACTER_SCL,CHARACTER_SCL }, { -12.f, 4.5f, 5.f }, 1.f, Quaternion{ 60.f,Vector3{0.f,1.f,0.f} });
    //CreateObject("character(L) 4", MeshID::GIRL_LEFTY, ImageID::WOOD_TEX_1, ColliderType::OBB, Vec3{ CHARACTER_SCL,CHARACTER_SCL,CHARACTER_SCL }, { -12.5f, 4.5f, -5.f }, 1.f, Quaternion{ 60.f,Vector3{0.f,1.f,0.f} });

    //(5) MIRROR
    Vec3 mirrorColliderSize = Vec3{ 7.5f,7.5f,0.5f };
    m_mirror=CreateObject("planar mirror", MeshID::PLANE, ImageID::MIRROR_TEX, ColliderType::OBB, mirrorColliderSize, {0.f, MIRROR_POS_Y, -4.5f}, 1.f, Quaternion{ 180.f,Vector3{0.f,1.f,0.f} },ObjectType::REFLECTIVE_FLAT);

    //(6) IDOL
    constexpr float IDOL_SCL = 9.f;
    m_idol = CreateObject("idol", MeshID::GRIM_REAPER_LEFTY, ImageID::SPHERE_TEX, ColliderType::OBB, Vector3{ IDOL_SCL,IDOL_SCL,IDOL_SCL }, { -0.5f, 5.3f, 0.5f }, 15.f, Quaternion{}, ObjectType::REFLECTIVE_CURVED);
}

void Core::Scene::ApplyBroadPhase()
{
    //TODO::Octree Partitioning 
    //https://www.gamedeveloper.com/programming/octree-partitioning-techniques
}

void Core::Scene::ApplyNarrowPhaseAndResolveCollisions(float dt)
{
    m_collisionManager.Reset();
    //ThreadPool& pool = ThreadPool::GetInstance();
    const size_t objSize = m_objects.size();
    
    // detect collisions in parallel
	for (size_t i{}; i < objSize; ++i) {
		for (size_t j{ i + 1 }; j < objSize; ++j) {
			// enqueue collision checks
            //pool.enqueue([this, i, j]() {
                m_collisionManager.CheckCollision(m_objects[i].get(), m_objects[j].get());
                //});
		}
	}    

    // resolve stored collisions
    m_collisionManager.ResolveCollision(dt);

    //deactivate knocked off objects
    RemoveObjectsBelowThreshold();
}

void Core::Scene::ShrinkPlaneOverTime(float dt) {
    Physics::Collider* collider = m_plane->GetCollider();

    if (auto scale = std::get_if<Vec3>(&collider->GetScale())) {
        // if the collider is a BoxCollider, scale is a Vec3
        Vec3 shrinkAmount = (*scale) *(PLANE_SHRINK_SPEED * dt); // calc the amount to shrink based on dt
        Vec3 newScale = (*scale) - shrinkAmount; // subtract the shrink amount from the current scale
        collider->SetScale(2.f*newScale); 
    }
    else if (auto radius = std::get_if<float>(&collider->GetScale())) {
        // if the collider is a SphereCollider, scale is a float representing the radius
        float shrinkAmount = (*radius) * (PLANE_SHRINK_SPEED * dt); // calc the amount to shrink based on dt
        float newRadius = (*radius) - shrinkAmount; // subtract the shrink amount from the current radius
        collider->SetScale(2.f*newRadius); 
    }
}

Rendering::MeshID Core::Scene::GetRandomIdolMeshID() const{
    static bool seedInitialized = false;
    if (!seedInitialized) {
        std::srand(std::time(nullptr)); 
        seedInitialized = true;
    }

    int firstSpecialID = static_cast<int>(MeshID::CAT);
    int lastSpecialID = static_cast<int>(MeshID::GRIM_REAPER_LEFTY);
    int randomID = firstSpecialID + std::rand() % (lastSpecialID - firstSpecialID + 1);

    return static_cast<MeshID>(randomID);
}


void Core::Scene::SetUpOrbitalLights() {
    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    //rendom offset between -1 and 1
    for (int i{}; i < m_numLights; ++i) {
        float offsetX = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX) * 2.f - 1.f;
        float offsetY = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX) * 2.f + 7.f;//1~3
        float offsetZ = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX) * 2.f - 1.f;
        m_orbitalLights[i].m_lightOrbitOffset = Vec3(offsetX,offsetY,offsetZ);

        // random orbital radius between 7.0 and 12.0
        float rad = 7.f + static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX) * 5.f;
        m_orbitalLights[i].m_orbitalRad = rad;

        // random orbital speed between 0.1 and 0.3
        float speed = 0.1f + static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX) * 0.2f;
        m_orbitalLights[i].m_orbitalSpeed = speed;

        m_orbitalLights[i].m_accumulatedTime = 0.f;

        float angle = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX) * 360.f; // in degrees
        m_orbitalLights[i].m_rotationAngle = angle;

        // random light intensity for each color channel
        const float minIntensity = 0.3f;
        float intensityR = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX)*0.5f + minIntensity;
        float intensityG = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX)*0.5f + minIntensity;
        float intensityB = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX)*0.5f + minIntensity;
        m_orbitalLights[i].m_intensity = Vec4(intensityR, intensityG, intensityB, 1.f);
    }
}




//Integrated the projectiles directly into the m_objects vector within the Scene class to not alter the whole rendering process. 
//the key is to manage these projectile objects effectively within the existing structure just for the demo.
void Core::Scene::SetUpProjectiles() {
    static constexpr float PROJECTILE_SCL = 1.f;
    m_projectiles.clear();

    std::random_device rd;
    std::mt19937 gen(rd());

    std::uniform_int_distribution<> meshDist(static_cast<int>(MeshID::CUBE), static_cast<int>(MeshID::GOURD));
    std::uniform_int_distribution<> imageDist(0, static_cast<int>(ImageID::POTTERY_TEX_3));

    for (int i{}; i < NUM_PROJECTILES; ++i) {
        MeshID randomMeshID = static_cast<MeshID>(meshDist(gen));
        ImageID randomImageID = static_cast<ImageID>(imageDist(gen));

        ColliderType colliderType;
        ColliderConfig colliderConfig;

        //// Sphere collider for Sphere and Dodecahedron meshes
        //if (randomMeshID == MeshID::SPHERE || randomMeshID == MeshID::DODECAHEDRON) {
        //    colliderType = ColliderType::SPHERE;
        //    colliderConfig = PROJECTILE_SCL; // sphere radius
        //}
        //else {
            colliderType = ColliderType::OBB;
            colliderConfig = Vec3{ PROJECTILE_SCL, PROJECTILE_SCL, PROJECTILE_SCL }; // box scale
        //}

        Core::Object* projectile = CreateObject(
            "projectile" + std::to_string(i),
            randomMeshID,
            randomImageID,
            colliderType,
            colliderConfig,
            { mainCam.GetPos().x, mainCam.GetPos().y, mainCam.GetPos().z },
            Projectile::PROJECTILE_MASS, // mass
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
