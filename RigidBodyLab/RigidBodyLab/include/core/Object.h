#pragma once
#include <math/Math.h>
#include <utilities/ToUnderlyingEnum.h>
#include <rendering/Mesh.h>
#include <rendering/ResourceManager.h>
#include <physics/RigidBody.h>
#include <physics/Collider.h>
#include <memory>
#include <vector>
#include <variant>

namespace Core {
	using namespace Rendering;
	using namespace Physics;

	class Object {
	private:
		ImageID m_imageID;

		//Dependency Injection
		const Mesh* m_mesh;     
		std::unique_ptr<Collider> m_collider;
		//'Rigidbody' for dynamic objects, 'Transform' for static objects
		std::variant<std::unique_ptr<RigidBody>, Transform> m_physicsOrTransform;
		
		// Offset to adjust the position of the mesh. This helps in visually aligning the mesh with the collider
		// especially useful when the physical collider shape and the visual mesh do not perfectly align, like a vase.
		Vec3 m_meshOffset;
	public:
		// Constructor for static objects (only Mat4 needed)
		Object(const Mesh* mesh, ImageID imageID, std::unique_ptr<Collider> collider, const Transform& transform, const Vec3 meshOffset = Vec3{0.f,0.f,0.f})
			: m_mesh(mesh), m_imageID(imageID), m_collider(std::move(collider)), m_physicsOrTransform(transform), m_meshOffset{meshOffset} {}

		// Constructor for dynamic objects
		Object(const Mesh* mesh, ImageID imageID, std::unique_ptr<Collider> collider, std::unique_ptr<RigidBody> rigidBody, const Vec3 meshOffset = Vec3{ 0.f,0.f,0.f })
			: m_mesh(mesh), m_imageID(imageID), m_collider(std::move(collider)), m_physicsOrTransform(std::move(rigidBody)),m_meshOffset{meshOffset} {}

		Object(const Object& other) = default;
		Object(Object&& other) noexcept = default;
		Object& operator=(const Object& other) = default;
		Object& operator=(Object&& other) noexcept = default;

		// Getter methods (setters might not be necessary because we are passing by reference)
		Vector3 GetPosition()const;
		Vector3 GetAxis(int axisIdx) const;
		const Mesh* GetMesh() const { return m_mesh; }
		void SetMesh(const Mesh* mesh) { m_mesh = mesh; }
		//RT only (no scale)
		Matrix4 GetModelMatrix() const;
		Mat4 GetUnitModelMatrixGLM() const;
		Mat4 GetModelMatrixGLM() const;
		const Collider* GetCollider() const;
		RigidBody* GetRigidBody();
		const RigidBody* GetRigidBody() const;

		bool IsDynamic() const;


		ImageID GetImageID() const { return m_imageID; }
		void SetImageID(ImageID id) { m_imageID = id; }

		void Integrate(float deltaTime);
	};

}
