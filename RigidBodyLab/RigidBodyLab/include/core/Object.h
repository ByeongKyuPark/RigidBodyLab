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
		Mesh& m_mesh;     
		std::unique_ptr<Collider> m_collider;
		//'Rigidbody' for dynamic objects, 'Mat4' for static objects
		std::variant<std::unique_ptr<RigidBody>, Mat4> m_physicsOrTransform;

	public:
		// Constructor for static objects (only Mat4 needed)
		Object(Mesh& mesh, ImageID imageID, std::unique_ptr<Collider> collider, const Mat4& transform)
			: m_mesh(mesh), m_imageID(imageID), m_collider(std::move(collider)), m_physicsOrTransform(transform) {}

		// Constructor for dynamic objects
		Object(Mesh& mesh, ImageID imageID, std::unique_ptr<Collider> collider, std::unique_ptr<RigidBody> rigidBody)
			: m_mesh(mesh), m_imageID(imageID), m_collider(std::move(collider)), m_physicsOrTransform(std::move(rigidBody)) {}

		Object(const Object& other) = default;
		Object(Object&& other) noexcept = default;
		Object& operator=(const Object& other) = default;
		Object& operator=(Object&& other) noexcept = default;

		// Getter methods (setters might not be necessary because we are passing by reference)
		const Mesh& GetMesh() const { return m_mesh; }
		Mat4 GetModelSRTMatrix() const;
		Mat4 GetModelRTMatrix() const;
		const Collider* GetCollider() const;
		const Physics::RigidBody* Core::Object::GetRigidBody() const;

		bool IsDynamic() const;


		ImageID GetImageID() const { return m_imageID; }
		void SetImageID(ImageID id) { m_imageID = id; }

		void Integrate(float deltaTime);
	};

}
