//#pragma once
//
//#include "Object.h"
//#include <vector>
//
//namespace Rendering {
//
//    class Scene {
//    private:
//        std::vector<Object> m_objects;
//        Mesh m_meshes[MeshID::NumMeshes];
//        Vec3 m_lightPositionsWF[NUM_LIGHTS], m_lightPositionsVF[NUM_LIGHTS];
//        Vec4 m_ambientAlbedo;
//        Vec4 m_diffuseAlbedo;
//        Vec4 m_specularAlbedo;
//        int m_specularPower;
//        Vec3 m_mirrorTranslate;
//        Vec3 m_mirrorRotationAxis;
//        float m_mirrorRotationAngle;
//        Vec3 m_spherePos;
//
//    public:
//        Scene() : m_specularPower(10) { /* Initialize default scene values here */ }
//
//        // Add an object to the scene
//        void AddObject(const Object& object) {
//            m_objects.push_back(object);
//        }
//
//        // Remove an object from the scene by index
//        void RemoveObject(size_t index) {
//            if (index < m_objects.size()) {
//                m_objects.erase(m_objects.begin() + index);
//            }
//        }
//
//        // Scene setup (initialization)
//        void SetUpScene() {
//            // Implementation for setting up the scene
//        }
//
//        // Getters and setters
//        const std::vector<Object>& GetObjects() const { return m_objects; }
//        void SetObjects(const std::vector<Object>& objects) { m_objects = objects; }
//
//        Vec3 GetMirrorTranslation() const { return m_mirrorTranslate; }
//        void SetMirrorTranslation(const Vec3& translation) { m_mirrorTranslate = translation; }
//
//        Vec3 GetMirrorRotationAxis() const { return m_mirrorRotationAxis; }
//        void SetMirrorRotationAxis(const Vec3& axis) { m_mirrorRotationAxis = axis; }
//
//        float GetMirrorRotationAngle() const { return m_mirrorRotationAngle; }
//        void SetMirrorRotationAngle(float angle) { m_mirrorRotationAngle = angle; }
//
//        Vec3 GetSpherePosition() const { return m_spherePos; }
//        void SetSpherePosition(const Vec3& position) { m_spherePos = position; }
//
//        // ... Additional getters and setters for other properties ...
//
//        // Methods to manage lighting, materials, etc.
//        // ...
//
//        // Any other scene-related methods...
//    };
//
//}
