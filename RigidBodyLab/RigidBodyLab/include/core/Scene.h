#pragma once

#include <core/Object.h>
#include <core/Projectile.h>
#include <physics/CollisionData.h>
#include <physics/CollisionManager.h>
#include <utilities/ThreadPool.h>
#include <vector>
#include <variant>

namespace Core {
    using Physics::CollisionData;
    using Physics::CollisionManager;

    class Scene {
        static constexpr int NUM_LIGHTS = 1;
        static constexpr int NUM_PROJECTILES = 50;

        std::vector<std::unique_ptr<Core::Object>> m_objects;
        std::vector<Projectile> m_projectiles;
        /*  Light pos are defined in world frame, but we need to compute their pos in view frame for
            lighting. In this frame, the vertex positions are not too large, hence the computation
            is normally more accurate.
        */
        Vec3 m_lightPosWF[NUM_LIGHTS];
        Vec3 m_lightPosVF[NUM_LIGHTS];

        Vec4 m_I;
        Vec4 m_ambientAlbedo;
        Vec4 m_diffuseAlbedo;
        Vec4 m_specularAlbedo;
        int m_specularPower;

        CollisionManager m_collisionManager;
        ThreadPool m_threadPool;
        //Special objects require seperate rendering 
        const Core::Object* m_mirror;//planar mirror
        const Core::Object* m_sphere;//spherical mirror (or refraction)

    private:
        //Integrated the projectiles directly into the m_objects vector within the Scene class,
        //so as not to alter the whole rendering process. 
        //the key is to manage these projectile objects effectively within the existing structure just for the demo.
        void SetUpProjectiles();
        void SetUpLight(float height);
        void SetUpScene();
        void ApplyBroadPhase();
        void ApplyNarrowPhaseAndResolveCollisions(float dt);

        friend class Renderer;

    public:
        Scene();

        void Update(float dt);

        Core::Object& GetObject(size_t index);
        const Core::Object& GetObject(size_t index) const;
        const Vec3* GetLightPositionsWF() const { return m_lightPosWF; }
        const Vec4& GetIntensity() const { return m_I; }
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
            const Quaternion & orientation = Quaternion{},
            ObjectType objType = ObjectType::REGULAR,
            bool isCollisionEnabled=true
            );

        void ShootProjectile(const Vector3& position);
        void ReloadProjectiles();
    };
}
