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

	enum class ObjectType {
		REFLECTIVE_FLAT,    // Planar Mirror
		REFLECTIVE_CURVED,  // Spherical Mirror
		MAPPABLE_PLANE,      // Plane with normal and parallax mapping capabilities
		REGULAR            // Normal Objects
	};

	class Object {
	private:
		ImageID m_imageID;
		ObjectType m_objType;

		//Dependency Injection
		const Mesh* m_mesh;     //not owner
		std::unique_ptr<Collider> m_collider; //owner
		//'Rigidbody' for dynamic objects, 'Transform' for static objects
		std::variant<std::unique_ptr<RigidBody>, Transform> m_physicsOrTransform; //owner
		std::string m_name;
	public:
		// Constructor for static objects (only Mat4 needed)
		Object(const std::string& name,const Mesh* mesh, ImageID imageID, std::unique_ptr<Collider> collider, const Transform& transform, ObjectType _type)
			: m_name{ name }, m_mesh(mesh), m_imageID(imageID), m_collider(std::move(collider)), m_physicsOrTransform(transform), m_objType{_type} {}

		// Constructor for dynamic objects
		Object(const std::string& name, const Mesh* mesh, ImageID imageID, std::unique_ptr<Collider> collider, std::unique_ptr<RigidBody> rigidBody, ObjectType _type)
			:m_name{ name }, m_mesh(mesh), m_imageID(imageID), m_collider(std::move(collider)), m_physicsOrTransform(std::move(rigidBody)), m_objType{_type} {}

		Object(const Object& other) = default;
		Object(Object&& other) noexcept = default;
		Object& operator=(const Object& other) = default;
		Object& operator=(Object&& other) noexcept = default;

		void SetMesh(const Mesh* mesh) { m_mesh = mesh; }
		void SetImageID(ImageID id) { m_imageID = id; }
		
		// Getter methods (setters might not be necessary because we are passing by reference)
		Vector3 GetPosition()const;
		Vector3 GetAxis(int axisIdx) const;
		const Mesh* GetMesh() const { return m_mesh; }
		//RT only (no scale)
		Matrix4 GetUnitModelMatrix() const;
		Mat4 GetModelMatrix() const;
		const Collider* GetCollider() const;
		Collider* GetCollider();
		RigidBody* GetRigidBody();
		const RigidBody* GetRigidBody() const;
		std::string GetName() const { return m_name; }
		ImageID GetImageID() const { return m_imageID; }
		ObjectType GetObjType() const { return m_objType; }

		bool IsDynamic() const;
		void Integrate(float deltaTime);
	};

}
