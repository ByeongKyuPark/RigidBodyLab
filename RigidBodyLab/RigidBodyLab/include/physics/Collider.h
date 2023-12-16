#pragma once
#include <math/Math.h>
namespace Physics {

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
		virtual Mat4 GetScaleMatrix() const = 0;
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
		Mat4 GetScaleMatrix() const override final{
			return glm::scale(glm::mat4(1.0f), glm::vec3(scale.x, scale.y, scale.z));
		}
	};

	//(2) sphere collider
	class SphereCollider : public Collider {
		float diameter;
	public:
		SphereCollider(float _diameter) : diameter(_diameter) {}

		bool IsCollidingWith(const Collider& other) const override final {
			return false;
		}
		void SetScaleInternal(float scale) {
			diameter = scale;
		}
		Mat4 GetScaleMatrix() const override final{
			return glm::scale(glm::mat4(1.0f), glm::vec3(diameter, diameter, diameter));
		}
	};

}