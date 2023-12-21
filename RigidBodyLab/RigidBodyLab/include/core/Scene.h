#pragma once

#include <core/Object.h>
#include <physics/CollisionData.h>
#include <physics/CollisionManager.h>
#include <vector>

namespace Core {
    using Physics::CollisionData;
    using Physics::CollisionManager;

    class Scene {
        static constexpr int NUM_LIGHTS = 1;

        std::vector<std::unique_ptr<Core::Object>> m_objects;

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
        
        //Special objects require seperate rendering 
        const Core::Object* m_mirror;//planar mirror
        const Core::Object* m_sphere;//spherical mirror (or refraction)

    private:
        //void UpdateLightPosViewFrame();
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
        Object* CreateObject(
            const std::string& name,
            MeshID meshID,
            ImageID textureID,
            ColliderType colliderType,
            const Vec3& scale = {1.f,1.f,1.f},
            const Vector3& position = Vector3{0.f,0.f,0.f},
            const Quaternion& orientation = Quaternion{},
            float mass=0.f
        )
        {

            // Fetch the mesh and texture
            auto mesh = ResourceManager::GetInstance().GetMesh(meshID);

            if (mass!=0.f) {
                m_objects.emplace_back(std::make_unique<Core::Object>(
                    name, 
                    mesh, 
                    textureID, 
                    std::make_unique<BoxCollider>(scale),
                    std::make_unique<RigidBody>(Transform{ position,orientation }, mass,colliderType)
                    ));
            }
            else {
                m_objects.emplace_back(std::make_unique<Core::Object>(
                    name,
                    mesh,
                    textureID,
                    std::make_unique<BoxCollider>(scale),
                    Transform{ position,orientation }
                    ));
            }
            return m_objects.back().get();
        }
    };
}
