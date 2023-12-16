#include <math/Vector3.h>
namespace Physics{

	using Math::Vector3;

	//abstract class
	class Collider {
	public:
	    virtual bool IsCollidingWith(const Collider& other) const = 0;
	};

	//(1) box collider
	class BoxCollider : public Collider {
	    Vector3 size; // the dimensions of the box

	public:
	    BoxCollider(const Vector3& size) : size(size) {}

	    bool IsCollidingWith(const Collider& other) const override final{
	    }
	};
	
	//(2) sphere collider
	class SphereCollider : public Collider {
    float radius;

	public:
	    SphereCollider(float radius) : radius(radius) {}

	    bool IsCollidingWith(const Collider& other) const override final{
	    }
	};

}