#pragma once
#include <math/Math.h>
namespace Physics{

	//abstract class
	class Collider {
	public:
	    virtual bool IsCollidingWith(const Collider& other) const = 0;
	};

	//(1) box collider
	class BoxCollider : public Collider {
	    Vec3 size; // the dimensions of the box

	public:
	    BoxCollider(const Vec3& size) : size(size) {}

	    bool IsCollidingWith(const Collider& other) const override final{
			return false;
	    }
	};
	
	//(2) sphere collider
	class SphereCollider : public Collider {
    float radius;

	public:
	    SphereCollider(float radius) : radius(radius) {}

	    bool IsCollidingWith(const Collider& other) const override final{
			return false;
	    }
	};

}