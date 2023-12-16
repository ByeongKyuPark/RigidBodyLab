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
        std::array<Mesh, TO_INT(MeshID::NUM_MESHES)> meshes;
		std::array<GLuint, TO_INT(ImageID::NUM_IMAGES)> textureIDs;
		GLuint bumpTexID, normalTexID;
		GLuint skyboxTexID;
		/*  For generating sphere "reflection/refraction" texture */
		GLuint sphereTexID;
		/*  For generating mirror "reflection" texture */
		GLuint mirrorTexID;
		GLuint mirrorFrameBufferID;
		
		int skyboxFaceSize;


		static constexpr char* objTexFile[TO_INT(ImageID::NUM_IMAGES)] = { "../RigidBodyLab/images/stone.png", "../RigidBodyLab/images/wood.png", "../RigidBodyLab/images/pottery.jpg" };
		/*  For bump/normal texture */
		static constexpr char* bumpTexFile = "../RigidBodyLab/images/stone_bump.png";
		/*  For environment texture */
		static constexpr char* skyboxTexFile = "../RigidBodyLab/images/skybox.jpg";

		friend class Renderer;
	public:
        ResourceManager();
		static ResourceManager& GetInstance();

        Mesh& GetMesh(MeshID id);
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