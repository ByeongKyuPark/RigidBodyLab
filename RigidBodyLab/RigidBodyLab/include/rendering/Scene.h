#pragma once

#include <core/Object.h>
#include <physics/CollisionData.h>
#include <physics/CollisionManager.h>
#include <vector>
#include <optional>

namespace Rendering {
    using Physics::CollisionData;
    using Physics::CollisionManager;

    class Scene {
        static constexpr int NUM_LIGHTS = 1;
        static constexpr float SPHERE_INERTIA_FACTOR = 0.4f;
        static constexpr float CUBE_INERTIA_FACTOR = 1 / 6.0f;

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

        /*  Mirror and sphere positions, which are used in graphics.cpp for rendering scene from these objects */
        Vec3 m_mirrorTranslate;
        Vec3 m_mirrorRotationAxis;
        float m_mirrorRotationAngle;
        Vec3 m_spherePos;

        CollisionManager m_collisionManager;

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
    };
}
