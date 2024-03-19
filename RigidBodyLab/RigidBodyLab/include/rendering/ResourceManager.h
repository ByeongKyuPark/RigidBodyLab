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
		CAT,
		CHARACTER,
		GRIM_REAPER,
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
		RIPPLE,
		MIRROR_TEX,
		SPHERE_TEX,
		NUM_IMAGES
	};

	/*  6 faces of the texture cube */
	enum class CubeFaceID {
		RIGHT = 0, LEFT, TOP, BOTTOM, BACK, FRONT, NUM_FACES
	};


    class ResourceManager {

		std::array<std::unique_ptr<Mesh>, TO_INT(MeshID::NUM_MESHES)> m_meshes;
		std::array<GLuint, TO_INT(ImageID::NUM_IMAGES)> m_textureIDs;

		std::unique_ptr<unsigned char[]> m_sphereCubeMapData[TO_INT(CubeFaceID::NUM_FACES)];

		GLuint m_bumpTexID, m_normalTexID;
		GLuint m_skyboxTexID;
		/*  For generating sphere "reflection/refraction" texture */
		GLuint m_sphereTexID;
		/*  For generating mirror "reflection" texture */
		GLuint m_mirrorTexID;
		GLuint m_mirrorFrameBufferID;
		
		int m_skyboxFaceSize;

		static constexpr char* OBJ_TEXTURES_PATH[TO_INT(ImageID::NUM_IMAGES)] = { 
			"../RigidBodyLab/images/stone.png", 
			"../RigidBodyLab/images/stone2.jpg",
			"../RigidBodyLab/images/wood.png",
			"../RigidBodyLab/images/wood_2.jpg",
			"../RigidBodyLab/images/pottery.jpg",
			"../RigidBodyLab/images/pottery_2.png",
			"../RigidBodyLab/images/pottery_3.jpg",
			"../RigidBodyLab/images/ripple.jpg"
		};
		
		/*  For bump/normal texture */
		static constexpr char* BUMP_TEXTURE_PATH = "../RigidBodyLab/images/stone_bump.png";
		/*  For environment texture */
		static constexpr char* SKYBOX_TEXTURE_PATH = "../RigidBodyLab/images/skybox.jpg";
		static constexpr char* SEPARATE_SKYBOX_TEXTURE_PATH[6] = {
			"../RigidBodyLab/images/MarriottMadisonWest/posx.jpg", // Right
			"../RigidBodyLab/images/MarriottMadisonWest/negx.jpg", // Left
			"../RigidBodyLab/images/MarriottMadisonWest/posy.jpg", // Top
			"../RigidBodyLab/images/MarriottMadisonWest/negy.jpg", // Bottom
			"../RigidBodyLab/images/MarriottMadisonWest/posz.jpg", // Back
			"../RigidBodyLab/images/MarriottMadisonWest/negz.jpg"  // Front
		};
		void FlipImageVertically(unsigned char* imageData, int width, int height, int numComponents)
		{
			int rowSize = width * numComponents;
			auto* tempRow = new unsigned char[rowSize];
			for (int y = 0; y < height / 2; ++y)
			{
				memcpy(tempRow, &imageData[y * rowSize], rowSize);
				memcpy(&imageData[y * rowSize], &imageData[(height - 1 - y) * rowSize], rowSize);
				memcpy(&imageData[(height - 1 - y) * rowSize], tempRow, rowSize);
			}
			delete[] tempRow;
		}
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
		/**
		 * Initializes the cube map texture data for the spherical mirror.
		 * This function allocates memory for each face of the cube map (unique_ptr),
		 * based on the size specified for the skybox. It should be called
		 * after the skybox texture is set up, ensuring that the skybox size
		 * is correctly initialized before allocating memory for the spherical
		 * mirror's cube map texture.
		 *
		 * Each face of the cube map is allocated memory equivalent to
		 * the skybox face size, with each pixel occupying 4 bytes (presumably for RGBA).
		 */
		void InitSphericalMirrorTexture();

		void SetUpObjTextures();
		void SetUpBaseBumpNormalTextures();
		void SetTextureParameters(GLenum textureType);
		void SetUpPlanarMirrorTexture();
		void SetUpSkyBoxTexture();
		void SetUpSeparateSkyBoxTexture();

		void SetUpSphereCubeMapTexture();

		void CopySubTexture(unsigned char* destTex, const unsigned char* srcTex,
			int size, int imgWidth,
			int verticalOffset, int horizontalOffset,
			bool verticalFlip, bool horizontalFlip,
			int numComponents);
		void Bump2Normal(const unsigned char* bumpImg, unsigned char* normalImg, int width, int height);
    };
}