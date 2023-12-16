#pragma once

#include <core/Object.h>
#include <vector>

namespace Rendering {
    class Scene {
        static constexpr int NUM_LIGHTS = 1;

        std::vector<Core::Object> m_objects;

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

    private:
        //void UpdateLightPosViewFrame();
        void SetUpLight(float height);
        void SetUpScene();

        friend class Renderer;

    public:
        Scene();

        Core::Object& GetObject(size_t index);
        const Core::Object& GetObject(size_t index) const;
        const Vec3* GetLightPositionsWF() const { return m_lightPosWF; }
        const Vec4& GetIntensity() const { return m_I; }
    };
}
