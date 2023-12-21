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
		TEAPOT,
		DIAMOND,
		DODECAHEDRON,
		GOURD,
		PYRAMID,
		NUM_MESHES
	};

	/*  The ID for texture loading */
	enum class ImageID
	{
		STONE_TEX_1 = 0,
		STONE_TEX_2,
		WOOD_TEX_1,
		WOOD_TEX_2,
		POTTERY_TEX_1,
		POTTERY_TEX_2,
		POTTERY_TEX_3,
		MIRROR_TEX,
		SPHERE_TEX,
		NUM_IMAGES
	};


	/*  Types of objects */
	enum class ObjID {
		BASE = 0,
		VASE,
		MIRROR,
		//MIRRORBASE1,
		//MIRRORBASE2,
		//MIRRORBASE3,
		SPHERE,
		NUM_OBJS
	};

	/*  6 faces of the texture cube */
	enum class CubeFaceID {
		RIGHT = 0, LEFT, TOP, BOTTOM, BACK, FRONT, NUM_FACES
	};


    class ResourceManager {
		std::array<std::unique_ptr<Mesh>, TO_INT(MeshID::NUM_MESHES)> m_meshes;
		std::array<GLuint, TO_INT(ImageID::NUM_IMAGES)> m_textureIDs;

		GLuint m_bumpTexID, m_normalTexID;
		GLuint m_skyboxTexID;
		/*  For generating sphere "reflection/refraction" texture */
		GLuint m_sphereTexID;
		/*  For generating mirror "reflection" texture */
		GLuint m_mirrorTexID;
		GLuint m_mirrorFrameBufferID;
		
		int m_skyboxFaceSize;

		static constexpr char* objTexFile[TO_INT(ImageID::NUM_IMAGES)] = { 
			"../RigidBodyLab/images/stone.png", 
			"../RigidBodyLab/images/stone2.jpg",
			"../RigidBodyLab/images/wood.png",
			"../RigidBodyLab/images/wood_2.jpg",
			"../RigidBodyLab/images/pottery.jpg",
			"../RigidBodyLab/images/pottery_2.png",
			"../RigidBodyLab/images/pottery_3.jpg"
		};
/*  For bump/normal texture */
		static constexpr char* bumpTexFile = "../RigidBodyLab/images/stone_bump.png";
		/*  For environment texture */
		static constexpr char* skyboxTexFile = "../RigidBodyLab/images/skybox.jpg";

		friend class Renderer;
	public:
        ResourceManager();
		static ResourceManager& GetInstance();

		Mesh* ResourceManager::GetMesh(MeshID id);
		const Mesh* ResourceManager::GetMesh(MeshID id) const;
		void ResourceManager::SetMesh(MeshID id, std::unique_ptr<Mesh> newMesh);
		GLuint GetTexture(ImageID id);
		void SetUpTextures();
    private:
		void SetUpObjTextures();
		void SetUpBaseBumpNormalTextures();
		void SetUpMirrorTexture();
		void SetUpSkyBoxTexture();
		void SetUpSphereTexture(unsigned char* sphereCubeMapData[]);

		void CopySubTexture(unsigned char* destTex, const unsigned char* srcTex,
			int size, int imgWidth,
			int verticalOffset, int horizontalOffset,
			bool verticalFlip, bool horizontalFlip,
			int numComponents);
		void Bump2Normal(const unsigned char* bumpImg, unsigned char* normalImg, int width, int height);
    };
}