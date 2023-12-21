#pragma once
#include <math/Math.h>
#include <utilities/ToUnderlyingEnum.h>
#include <rendering/Mesh.h>
#include <rendering/ResourceManager.h>
#include <physics/RigidBody.h>
#include <physics/Collider.h>
#include <memory>
#include <vector>

namespace Core {
	using namespace Rendering;
	using namespace Physics;

	class Object {
	private:
		ImageID m_imageID;

		//Dependency Injection
		const Mesh* m_mesh;     //not owner
		std::unique_ptr<Collider> m_collider; //owner
		//'Rigidbody' for dynamic objects, 'Transform' for static objects
		std::variant<std::unique_ptr<RigidBody>, Transform> m_physicsOrTransform; //owner
		std::string m_name;
	public:
		// Constructor for static objects (only Mat4 needed)
		Object(const std::string& name,const Mesh* mesh, ImageID imageID, std::unique_ptr<Collider> collider, const Transform& transform)
			: m_name{name}, m_mesh(mesh), m_imageID(imageID), m_collider(std::move(collider)), m_physicsOrTransform(transform) {}

		// Constructor for dynamic objects
		Object(const std::string& name, const Mesh* mesh, ImageID imageID, std::unique_ptr<Collider> collider, std::unique_ptr<RigidBody> rigidBody)
			:m_name{ name }, m_mesh(mesh), m_imageID(imageID), m_collider(std::move(collider)), m_physicsOrTransform(std::move(rigidBody)) {}

		Object(const Object& other) = default;
		Object(Object&& other) noexcept = default;
		Object& operator=(const Object& other) = default;
		Object& operator=(Object&& other) noexcept = default;

		void SetMesh(const Mesh* mesh) { m_mesh = mesh; }
		
		// Getter methods (setters might not be necessary because we are passing by reference)
		Vector3 GetPosition()const;
		Vector3 GetAxis(int axisIdx) const;
		const Mesh* GetMesh() const { return m_mesh; }

		//RT only (no scale)
		Matrix4 GetModelMatrix() const;
		Mat4 GetUnitModelMatrixGLM() const;
		Mat4 GetModelMatrixGLM() const;
		const Collider* GetCollider() const;
		RigidBody* GetRigidBody();
		const RigidBody* GetRigidBody() const;

		bool IsDynamic() const;

		std::string GetName() const { return m_name; }
		ImageID GetImageID() const { return m_imageID; }
		void SetImageID(ImageID id) { m_imageID = id; }

		void Integrate(float deltaTime);
	};

}
