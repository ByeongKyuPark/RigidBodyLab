#pragma once
#include <math/Math.h>
#include <math/Matrix4.h>
#include <variant>
namespace Physics {

	using Math::Matrix4;

	using ColliderConfig = std::variant<float, Vec3>; // float for radius, Vec3 for scale

	enum class ColliderType {
		BOX=0,
		SPHERE
	};

	//pure abstract class (just an interface class)
	class Collider {
	public:
		template<typename T>
		void SetScale(const T& scale) {
			if constexpr (std::is_same_v<T, Vec3>) {
				static_cast<BoxCollider*>(this)->SetScaleInternal(scale);
			}
			else if constexpr (std::is_same_v<T, float>) {
				static_cast<SphereCollider*>(this)->SetScaleInternal(scale);
			}
		}
		virtual bool IsCollidingWith(const Collider& other) const = 0;
		virtual Mat4 GetScaleMatrixGLM() const = 0;
		virtual Matrix4 GetScaleMatrix() const = 0;

		// GetScale method that returns either a float or Vec3
		virtual std::variant<float, Vec3> GetScale() const = 0;
	};

	//(1) box collider
	class BoxCollider : public Collider {
		Vec3 scale; // the dimensions of the box
	public:
		BoxCollider(const Vec3& _scale) : scale(_scale) {}
		bool IsCollidingWith(const Collider& other) const override final {
			return false;
		}
		void SetScaleInternal(const Vec3& _scale) {
			scale = _scale;
		}
		Mat4 GetScaleMatrixGLM() const override final{
			return glm::scale(glm::mat4(1.0f), glm::vec3(scale.x, scale.y, scale.z));
		}
		Matrix4 GetScaleMatrix() const override final {
			return Matrix4{scale.x,scale.y,scale.z,1.f};
		}

		std::variant<float, Vec3> GetScale() const override final{
			return scale*0.5f; // Returns Vec3 for BoxCollider
		}
	};

	//(2) sphere collider
	class SphereCollider : public Collider {
		float radius;
	public:
		SphereCollider(float _radius) : radius(_radius) {}

		bool IsCollidingWith(const Collider& other) const override final {
			return false;
		}
		void SetScaleInternal(float scale) {
			radius = scale;
		}
		Mat4 GetScaleMatrixGLM() const override final{
			return glm::scale(glm::mat4(1.0f), glm::vec3(radius, radius, radius));
		}
		Matrix4 GetScaleMatrix() const override final {
			return Matrix4{ radius,radius,radius,1.f };
		}

		std::variant<float, Vec3> GetScale() const override final{
			return radius*0.5f; // Returns float (radius) for SphereCollider
		}
	};

	////(3) Infinite Plane
	//class PlaneCollider : public Collider {
	//	Vec3 normal;  // Normal vector of the plane
	//	float offset; // Distance from the origin

	//public:
	//	PlaneCollider(const Vec3& _normal, float _offset) : normal(_normal), offset(_offset) {}

	//	bool IsCollidingWith(const Collider& other) const override {
	//		return false;
	//	}

	//	Mat4 GetScaleMatrixGLM() const override final{
	//		//the scaling component is neutral and doesn't affect the plane's representation
	//		return glm::mat4(1.0f);
	//	}
	//	std::variant<float, Vec3> GetScale() const override final {
	//		//the scaling component is neutral and doesn't affect the plane's representation
	//		return 1.f;
	//	}
	//};
	//-----------------------------------
}