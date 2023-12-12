#pragma once
#include <array>
#include <utilities/ToUnderlyingEnum.h>
#include <rendering/Mesh.h>

namespace Rendering {

	class Object;

	/*  Pre-defined shapes: big flat cube, horizontal cube, vertical cube, sphere */
	enum class MeshID {
		CUBE = 0,
		VASE,
		PLANE,
		SPHERE,
		NUM_MESHES
	};

	/*  The ID for texture loading */
	enum class ImageID
	{
		STONE_TEX = 0,
		WOOD_TEX,
		POTTERY_TEX,
		MIRROR_TEX,
		SPHERE_TEX,
		NUM_IMAGES
	};


	/*  Types of objects */
	enum class ObjID {
		BASE = 0,
		VASE,
		MIRROR,
		MIRRORBASE1,
		MIRRORBASE2,
		MIRRORBASE3,
		SPHERE,
		NUM_OBJS
	};

    class ResourceManager {
    public:
        ResourceManager();
        Mesh& GetMesh(MeshID id);

    private:
        std::array<Mesh, TO_INT(MeshID::NUM_MESHES)> meshes;
        // Initialize your meshes and images here
    };
}