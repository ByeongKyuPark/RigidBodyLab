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
		//Mat4 m_modelToWorldMatrix;   

		//Dependency Injection
		Mesh& m_mesh;     
		std::unique_ptr<RigidBody> m_rigidBody;
		std::unique_ptr<Collider> m_collider;

	public:
		Object::Object(Mesh& mesh, ImageID imageID, const Mat4& modelMat, RigidBody* rigidBody, Collider* collider)
			: m_mesh(mesh), m_imageID(imageID), //m_modelToWorldMatrix(modelMat),
			m_rigidBody(rigidBody), m_collider(collider) {}

		Object(const Object& other) = default;
		Object(Object&& other) noexcept = default;
		Object& operator=(const Object& other) = default;
		Object& operator=(Object&& other) noexcept = default;

		// Getter methods (setters might not be necessary because we are passing by reference)
		const Mesh& GetMesh() const { return m_mesh; }
		const Mat4& GetModelToWorldMatrix() const { return m_rigidBody->GetLocalToWorldMatrix();}
		//const Mat4& GetModelToWorldMatrix() const { return m_modelToWorldMatrix; }

		ImageID GetImageID() const { return m_imageID; }
		void SetImageID(ImageID id) { m_imageID = id; }

		void Integrate(float deltaTime);
	};

}
