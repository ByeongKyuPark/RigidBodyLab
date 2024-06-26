#pragma once

#include <core/Object.h>
#include <core/Projectile.h>
#include <rendering/OrbitalLight.h>
#include <physics/CollisionData.h>
#include <physics/CollisionManager.h>
#include <vector>
#include <future>
#include <variant>

namespace Core {
    using Physics::CollisionData;
    using Physics::CollisionManager;

    class Scene {
        static constexpr int NUM_PROJECTILES = 50;
        static constexpr int NUM_INITIAL_GIRLS = 4;//the # of girl statues on the platform
        static constexpr float Y_THRESHOLD = -10.0f; //either remove or reload objects that fall below this threshold
        static constexpr float PLANE_SHRINK_SPEED = 0.025f;

        std::vector<std::unique_ptr<Core::Object>> m_objects;
        std::vector<Projectile> m_projectiles;
        /*  Light pos are defined in world frame, but we need to compute their pos in view frame for
            lighting. In this frame, the vertex positions are not too large, hence the computation
            is normally more accurate.
        */

        //lights
        std::vector<OrbitalLight> m_orbitalLights; 

        Vec4 m_ambientLightIntensity;
        Vec4 m_ambientAlbedo;
        Vec4 m_diffuseAlbedo;
        Vec4 m_specularAlbedo;
        int m_specularPower;      //regular objects
        int m_numLights;

        CollisionManager m_collisionManager;
        //Special objects require seperate rendering 
        const Core::Object* m_mirror;//planar mirror
        Core::Object* m_idol;//idol (spherical mirror)
        Core::Object* m_plane; //shrinks over time
        int m_numGirls{ NUM_INITIAL_GIRLS };
    private:
        //Integrated the projectiles directly into the m_objects vector within the Scene class,
        //so as not to alter the whole rendering process. 
        //the key is to manage these projectile objects effectively within the existing structure just for the demo.
        void SetUpProjectiles();
        void SetUpScene();
        void SetUpOrbitalLights();
        void ApplyBroadPhase();
        void ApplyNarrowPhaseAndResolveCollisions(float dt);
        void ShrinkPlaneOverTime(float dt);
        MeshID GetRandomIdolMeshID()const;
        void RestoreTrueIdentities();
        bool OnlyFollowersLeft()const { return m_numGirls <= 0; }

        friend class Renderer;

    public:
        Scene();

        void Update(float dt);
        int AddLight();
        int RemoveLight();

        void SetLightPosition(const Vector3& lightPos, int lightIdx=0);
        void SetLightColor(const Vec4& color, int lightIdx = 0);

        Core::Object& GetObject(size_t index);
        const Core::Object& GetObject(size_t index) const;
        //const std::vector<Vec3>& GetLightPositionsWF() const { return m_lightPosWF; }
        const Vec4& GetLightColor(int idx) const;
        const Vec3& GetLightPosition(int lightIdx) const;
        int GetNumLights()const { return m_numLights; }
        /**
         * Creates and returns a pointer to a new Object with the specified parameters.
         *
         * The function utilizes a `std::variant` for the collider configuration, allowing
         * the flexibility to specify either a radius for a sphere collider or a scale
         * for a box collider. This approach streamlines the function signature and
         * ensures type safety and clarity in specifying collider properties.
         *
         * @param name The name of the object to be created.
         * @param meshID The ID of the mesh to be used for the object.
         * @param textureID The ID of the texture to be applied to the object.
         * @param colliderType The type of collider to be attached to the object (e.g., BOX, SPHERE).
         * @param colliderConfig A `std::variant` holding either a float for sphere collider's radius
         *                       or a Vec3 for box collider's scale, depending on `colliderType`.
         * @param position The initial position of the object in the world (defaulted to {0.f, 0.f, 0.f}).
         * @param mass The mass of the object (defaulted to 1.0f).
         * @param orientation The initial orientation of the object (defaulted to an identity quaternion).
         * @return A pointer to the newly created Object.
         *
         * Usage examples:
         *   For a box collider - ColliderConfig colliderConfig = Vec3{1.f, 1.f, 1.f};
         *   For a sphere collider - ColliderConfig colliderConfig = 1.0f;
         */
        Object* CreateObject(
            const std::string& name,
            MeshID meshID,
            ImageID textureID,
            ColliderType colliderType,
            ColliderConfig colliderConfig,
            const Vector3& position = Vector3{ 0.f,0.f,0.f },
            float mass = 1.f,
            const Quaternion& orientation = Quaternion{},
            ObjectType objType = ObjectType::DEFERRED_REGULAR,
            bool isCollisionEnabled = true,
            bool isVisible = true
            );

        void ShootProjectile(const Vector3& position);
        void ReloadProjectiles();
        void RemoveObjectsBelowThreshold();
        void RemoveProjectiles();
        void RemoveAndNullifySpecialObjects();
        void Reset();
    };
}
