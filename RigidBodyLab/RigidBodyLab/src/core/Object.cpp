#include <core/Object.h>
#include <math/Math.h>
#include <utilities/ToUnderlyingEnum.h>

using namespace Rendering;

// Getter methods (setters might not be necessary because we are passing by reference)

Physics::Vector3 Core::Object::GetPosition() const {
	if (IsDynamic()) {
		return std::get<std::unique_ptr<RigidBody>>(m_physicsOrTransform)->GetPosition();
	}
	else {
		return std::get<Transform>(m_physicsOrTransform).m_position;
	}
}
Physics::Vector3 Core::Object::GetAxis(int axisIdx) const {
	if (IsDynamic()) {
		return std::get<std::unique_ptr<RigidBody>>(m_physicsOrTransform)->GetAxis(axisIdx);
	}
	else {
		return std::get<Transform>(m_physicsOrTransform).GetAxis(axisIdx);
	}
}

Physics::Matrix4 Core::Object::GetUnitModelMatrix() const
{
	if (IsDynamic()) {
									// TR													
		return std::get<std::unique_ptr<RigidBody>>(m_physicsOrTransform)->GetLocalToWorldMatrix();
	}
	else {							// TR									
		return std::get<Transform>(m_physicsOrTransform).m_localToWorld;
	}
}


Mat4 Core::Object::GetModelMatrix() const {
	/*
	This function computes the model matrix for an object, transforming it from model space to world space.
	The model matrix is essential for representing the object's position, orientation, and scale in the 3D world.

	Calculation differs for dynamic and static objects:

	- Dynamic Objects: For objects with movement and orientation changes, the matrix is derived from the RigidBody's
	  transformation, combined with the collider's scale and mesh's bounding box matrix. This ensures accurate scaling
	  and positioning according to the physical representation.

	- Static Objects: For immovable objects, the matrix is calculated using the Transform component's matrix, combined
	  with the collider's scale and mesh's bounding box matrix.

	Example:
	Consider a vase with a bounding box in model space having extents in x: [-0.5, 0.5], y: [-0.2, 0.8], z: [-0.5, 0.5].
	If the scale factor is 10, treat the vase initially as a cube. To center it (currently centered at 0.3 = (-0.2 + 0.8) / 2),
	move it down by -0.3 along the y-axis. Then, scale down the cube based on extents [0.5, 0.5, 0.5] and apply the SRT
	(scale-rotate-translate, aka model to world) matrix as usual. The final matrix represents a composite transformation
	from the object's local space to world space, incorporating position, orientation, scale, and physical bounds.
	*/
	if (IsDynamic()) {
		// for dynamic objects, combine RigidBody's transformation with the collider's scale and mesh offset
		return std::get<std::unique_ptr<RigidBody>>(m_physicsOrTransform)->GetLocalToWorldMatrix()
			* m_collider->GetScaleMatrix()
			* m_mesh->GetBoundingBoxMat();
	}
	else {
		// for static objects, combine Transform's transformation with the collider's scale and mesh offset
		return std::get<Transform>(m_physicsOrTransform).m_localToWorld
			* m_collider->GetScaleMatrix() 
			* m_mesh->GetBoundingBoxMat();
	}
}

const Physics::Collider* Core::Object::GetCollider() const {
	return m_collider.get();
}

Physics::Collider* Core::Object::GetCollider(){
	return m_collider.get();
}

bool Core::Object::IsDynamic() const {
	return std::holds_alternative<std::unique_ptr<Physics::RigidBody>>(m_physicsOrTransform);
}

Physics::RigidBody* Core::Object::GetRigidBody()
{
	// Using std::visit to handle different types in the variant
	return std::visit([](auto&& arg) -> Physics::RigidBody * {
			using T = std::decay_t<decltype(arg)>;
			if constexpr (std::is_same_v<T, std::unique_ptr<Physics::RigidBody>>) {
				// If the variant holds a RigidBody, return it
				return arg.get();
			}
			else {
				// If the variant holds a different type (e.g., Mat4), return nullptr
				return nullptr;
			}
		}, m_physicsOrTransform);
}

const Physics::RigidBody* Core::Object::GetRigidBody() const
{
	// Using std::visit to handle different types in the variant
	return std::visit([](auto&& arg) -> const Physics::RigidBody* {
		using T = std::decay_t<decltype(arg)>;
	if constexpr (std::is_same_v<T, std::unique_ptr<Physics::RigidBody>>) {
		// If the variant holds a RigidBody, return it
		return arg.get();
	}
	else {
		// If the variant holds a different type (e.g., Mat4), return nullptr
		return nullptr;
	}
		}, m_physicsOrTransform);
}


void Core::Object::Integrate(float deltaTime) {
	if (IsDynamic()) {
		std::get<std::unique_ptr<RigidBody>>(m_physicsOrTransform)->Integrate(deltaTime);
	}
}
