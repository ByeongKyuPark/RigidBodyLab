#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <image_io.h>
#include <glad/glad.h>
#include <rendering/Camera.h>
#include <rendering/ResourceManager.h>

using namespace Rendering;

ResourceManager::ResourceManager()
	:m_meshes{}, m_textureIDs{}
{
    stbi_set_flip_vertically_on_load(true);
 
    m_meshes[TO_INT(MeshID::CUBE)] = std::make_unique<Mesh>(Mesh::CreateCube(1, 1, 1));
    m_meshes[TO_INT(MeshID::PLANE)] = std::make_unique<Mesh>(Mesh::CreatePlane(1, 1));
    m_meshes[TO_INT(MeshID::SPHERE)] = std::make_unique<Mesh>(Mesh::CreateSphere(16, 16));
    m_meshes[TO_INT(MeshID::VASE)] = std::make_unique<Mesh>(Mesh::LoadOBJMesh("../RigidBodyLab/models/vase.obj"));
    m_meshes[TO_INT(MeshID::TEAPOT)] = std::make_unique<Mesh>(Mesh::LoadOBJMesh("../RigidBodyLab/models/teapot.obj"));
    m_meshes[TO_INT(MeshID::TEAPOT)]->m_boundingBox.extents.x *= 2.f; 
    // adjust the teapot's bounding box. The original model has an elongated shape (oval), so we scale its x-dimension to achieve a more proportionate and visually pleasing appearance.
    m_meshes[TO_INT(MeshID::DIAMOND)] = std::make_unique<Mesh>(Mesh::LoadOBJMesh("../RigidBodyLab/models/diamond.obj"));
    m_meshes[TO_INT(MeshID::DODECAHEDRON)] = std::make_unique<Mesh>(Mesh::LoadOBJMesh("../RigidBodyLab/models/dodecahedron.obj"));
    m_meshes[TO_INT(MeshID::GOURD)] = std::make_unique<Mesh>(Mesh::LoadOBJMesh("../RigidBodyLab/models/gourd.obj"));
}

ResourceManager& ResourceManager::GetInstance()
{
    static ResourceManager instance;
    return instance;
}

Mesh* Rendering::ResourceManager::GetMesh(MeshID id) {
    return m_meshes[TO_INT(id)].get();
}

const Mesh* Rendering::ResourceManager::GetMesh(MeshID id) const{
    return m_meshes[TO_INT(id)].get();
}

void Rendering::ResourceManager::SetMesh(MeshID id, std::unique_ptr<Mesh> newMesh) {
    m_meshes[TO_INT(id)] = std::move(newMesh);
}

GLuint Rendering::ResourceManager::GetTexture(ImageID id) {
	return m_textureIDs[TO_INT(id)];
}

void Rendering::ResourceManager::SetUpTextures() {
    /*  Set up textures for objects in the scene */
    SetUpObjTextures();

    /*  Set up bump map and normal map for the base object */
    SetUpBaseBumpNormalTextures();

    /*  Set up skybox texture for background rendering */
    SetUpSkyBoxTexture();

    /*  Set up texture object for mirror reflection. This texture object hasn't stored any data yet.
        We will render the reflected data for this texture on the fly.
    */
    SetUpPlanarMirrorTexture();
}

/******************************************************************************/
/*!
\fn     void SetUpObjTextures()
\brief
Read texture images from files, then copy them to graphics memory.
These textures will be combined with light colors for the objects
in the scene.
*/
/******************************************************************************/

void Rendering::ResourceManager::SetUpObjTextures()
{
	glGenTextures(TO_INT(ImageID::NUM_IMAGES), m_textureIDs.data());

	unsigned char* imgData;
	int imgWidth, imgHeight, numComponents;

	/*  Mirror and sphere will not use existing textures so we'll set them up separately */
	size_t NUM_IMGS = TO_INT(ImageID::NUM_IMAGES);
	for (int i{}; i < NUM_IMGS; ++i)
		if (i != TO_INT(ImageID::MIRROR_TEX) && i != TO_INT(ImageID::SPHERE_TEX))
		{
			imgData = stbi_load(objTexFile[i], &imgWidth, &imgHeight, &numComponents, 0);
			if (!imgData) {
				std::cerr << "Reading " << objTexFile[i] << " failed.\n";
				exit(1);
			}

			/*  Bind corresponding texture ID */
			glBindTexture(GL_TEXTURE_2D, m_textureIDs[i]);

			/*  Copy image data to graphics memory */
			if (numComponents == 3)
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, imgWidth, imgHeight, 0,
					GL_RGB, GL_UNSIGNED_BYTE, imgData);
			else
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, imgWidth, imgHeight, 0,
					GL_RGBA, GL_UNSIGNED_BYTE, imgData);

			/*  Done with raw image data so delete it */
			stbi_image_free(imgData);

			/*  Generate texture mipmaps. */
			glGenerateMipmap(GL_TEXTURE_2D);

			/*  Set up texture behaviors */
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		}
}

/******************************************************************************/
/*!
\fn     void SetUpBaseBumpNormalTextures()
\brief
Set up the bump map and normal map for normal mapping and parallax mapping.
*/
/******************************************************************************/

void ResourceManager::SetUpBaseBumpNormalTextures() {
    int imgWidth, imgHeight, numComponents;

    unsigned char* bumpImgData;
    if (ReadImageFile(bumpTexFile, &bumpImgData, &imgWidth, &imgHeight, &numComponents) == 0) {
        std::cerr << "Reading " << bumpTexFile << " failed.\n";
        exit(1);
    }

    // (create normal image)
    // when we allocate memory using new[] and assign it to a std::unique_ptr with the array version,
    // by the default deleter will use delete[] to deallocate the memory. 
    std::unique_ptr<unsigned char[]> normalImgData(new unsigned char[imgWidth * imgHeight * 3]);
    Bump2Normal(bumpImgData, normalImgData.get(), imgWidth, imgHeight);

    // generate texture ID for bump image and copy it to GPU
    glGenTextures(1, &m_bumpTexID);
    glBindTexture(GL_TEXTURE_2D, m_bumpTexID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, imgWidth, imgHeight, 0, GL_RED, GL_UNSIGNED_BYTE, bumpImgData);

    stbi_image_free(bumpImgData);

    // Set up texture behaviors and generate mipmaps for bump texture
    SetTextureParameters(GL_TEXTURE_2D);

    // Generate texture ID for normal image and copy it to GPU
    glGenTextures(1, &m_normalTexID);
    glBindTexture(GL_TEXTURE_2D, m_normalTexID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, imgWidth, imgHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, normalImgData.get());

    // Set up texture behaviors and generate mipmaps for normal texture
    SetTextureParameters(GL_TEXTURE_2D);
}

void ResourceManager::SetTextureParameters(GLenum textureType) {
    glGenerateMipmap(textureType);
    glTexParameteri(textureType, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(textureType, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(textureType, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(textureType, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
}


/******************************************************************************/
/*!
\fn     void SetUpMirrorTexture()
\brief
Set up texture and frame buffer objects for rendering mirror reflection.
*/
/******************************************************************************/

void Rendering::ResourceManager::SetUpPlanarMirrorTexture()
{
	glGenTextures(1, &m_mirrorTexID);
	glBindTexture(GL_TEXTURE_2D, m_mirrorTexID);

	/*  Some graphics drivers don't support glTexStorage2D */
	//glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, mirrorCam.width, mirrorCam.height);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, mirrorCam.GetWidth(), mirrorCam.GetHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	/*  Calling glTexImage2D with levels = 0 will only generate one level of texture,
	so let's not use mipmaps here.
	The visual difference in this case is hardly recognizable.
	*/
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glGenFramebuffers(1, &m_mirrorFrameBufferID);
	glBindFramebuffer(GL_FRAMEBUFFER, m_mirrorFrameBufferID);

	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_mirrorTexID, 0);

    //create a renderbuffer for depth attachment
    GLuint depthRenderBuffer;
    glGenRenderbuffers(1, &depthRenderBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, depthRenderBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, mirrorCam.GetWidth(), mirrorCam.GetHeight());
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRenderBuffer);

    // check if framebuffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "Mirror Framebuffer is not complete!\n";
    }

    //reset
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Rendering::ResourceManager::InitSphericalMirrorTexture() {
    //SetUp the cube map for the spherical mirror
    const int numFaces = TO_INT(CubeFaceID::NUM_FACES);
    for (int i = 0; i < numFaces; ++i) {
        m_sphereCubeMapData[i] = std::make_unique<unsigned char[]>(m_skyboxFaceSize * m_skyboxFaceSize * 4);
    }
}

/******************************************************************************/
/*!
\fn     void SetUpSkyBoxTexture()
\brief
Set up the cubemap texture from the skybox image.
*/
/******************************************************************************/
void Rendering::ResourceManager::SetUpSkyBoxTexture()
{
    int imgWidth, imgHeight, numComponents;
    unsigned char* cubeImgData{ nullptr };

    if (ReadImageFile(skyboxTexFile, &cubeImgData, &imgWidth, &imgHeight, &numComponents) == 0) {
        std::cerr << "Reading " << skyboxTexFile << " failed.\n";
        exit(1);
    }


    m_skyboxFaceSize = imgHeight / 3;
    int imgSizeBytes = sizeof(unsigned char) * m_skyboxFaceSize * m_skyboxFaceSize * numComponents;

    // allocate memory for each cube face and copy subtextures
    std::unique_ptr<unsigned char[]> cubeFace[TO_INT(CubeFaceID::NUM_FACES)];
    for (int f = 0; f < TO_INT(CubeFaceID::NUM_FACES); ++f) {
        cubeFace[f] = std::make_unique<unsigned char[]>(imgSizeBytes);
    }

    /*  Copy the texture from the skybox image to 6 textures using CopySubTexture */
    /*  imgWidth is the width of the original image, while skyboxFaceSize is the size of each face */
    /*  The cubemap layout is as described in the assignment specs */
    CopySubTexture(cubeFace[TO_INT(CubeFaceID::FRONT)].get(), cubeImgData, m_skyboxFaceSize, imgWidth, m_skyboxFaceSize, m_skyboxFaceSize, true, true, numComponents);
    CopySubTexture(cubeFace[TO_INT(CubeFaceID::BOTTOM)].get(), cubeImgData, m_skyboxFaceSize, imgWidth, m_skyboxFaceSize, 0, false, false, numComponents);
    CopySubTexture(cubeFace[TO_INT(CubeFaceID::LEFT)].get(), cubeImgData, m_skyboxFaceSize, imgWidth, 0, m_skyboxFaceSize, true, true, numComponents);
    CopySubTexture(cubeFace[TO_INT(CubeFaceID::RIGHT)].get(), cubeImgData, m_skyboxFaceSize, imgWidth, 2 * m_skyboxFaceSize, m_skyboxFaceSize, true, true, numComponents);
    CopySubTexture(cubeFace[TO_INT(CubeFaceID::TOP)].get(), cubeImgData, m_skyboxFaceSize, imgWidth, m_skyboxFaceSize, 2 * m_skyboxFaceSize, false, false, numComponents);
    CopySubTexture(cubeFace[TO_INT(CubeFaceID::BACK)].get(), cubeImgData, m_skyboxFaceSize, imgWidth, 3 * m_skyboxFaceSize, m_skyboxFaceSize, true, true, numComponents);

    glGenTextures(1, &m_skyboxTexID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_skyboxTexID);


    /*  Copy the 6 textures to the GPU cubemap texture object, and set appropriate texture parameters */
    GLuint internalFormat, format;
    if (numComponents == 3)
    {
        internalFormat = GL_RGB8;
        format = GL_RGB;
    }
    else
    {
        internalFormat = GL_RGBA8;
        format = GL_RGBA;
    }
    // Upload each face of the cubemap
    for (int f = 0; f < TO_INT(CubeFaceID::NUM_FACES); ++f)
    {
        // GL_TEXTURE_CUBE_MAP_POSITIVE_X + f corresponds to the cube map face target
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + f, 0, internalFormat, m_skyboxFaceSize, m_skyboxFaceSize, 0, format, GL_UNSIGNED_BYTE, cubeFace[f].get());
    }

    // Set the texture parameters
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    // Generate mipmaps for the cubemap
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    free(cubeImgData);
    //for (int f = 0; f < TO_INT(CubeFaceID::NUM_FACES); ++f) {
    //    free(cubeFace[f].get());
    //}

    InitSphericalMirrorTexture();
}

/******************************************************************************/
/*!
\fn     void SetUpSphereTexture()
\brief
Set up texture object for rendering sphere reflection/refraction.
*/
/******************************************************************************/

void Rendering::ResourceManager::SetUpSphereCubeMapTexture()
{
    glGenTextures(1, &m_sphereTexID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_sphereTexID);

    for (int f = 0; f < TO_INT(CubeFaceID::NUM_FACES); ++f)
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + f, 0, GL_RGBA8, m_skyboxFaceSize, m_skyboxFaceSize,
            0, GL_RGBA, GL_UNSIGNED_BYTE, m_sphereCubeMapData[f].get());

    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
    //this helps in smoothing out the discontinuities at the edges of the cube map faces.
}

/******************************************************************************/
/*!
\fn     void CopySubTexture(    unsigned char *destTex, const unsigned char *srcTex,
int size, int imgWidth,
int verticalOffset, int horizontalOffset,
bool verticalFlip, bool horizontalFlip,
int numComponents)
\brief
Copy from an area from srcTex to destTex.
The destTex has dimensions size*size.
The columns in srcTex will be from verticalOffset to verticalOffset + size - 1.
The rows in srcTex will be from horizontalOffset to horizontalOffset + size - 1.
If verticalFlip/horizontalFlip is true then we have to flip the columns/rows within
the copied range vertically/horizontally.
\param  size
Dimension of the copied area.
\param  imgWidth
Width of srcTex.
\param  verticalOffset
First column of the copied area in srcTex.
\param  horizontalOffset
First row of the copied area in srcTex.
\param  verticalFlip
Whether we need to flip the columns within the copied area.
\param  horizontalFlip
Whether we need to flip the rows within the copied area.
*/
/******************************************************************************/

void Rendering::ResourceManager::CopySubTexture(unsigned char* destTex, const unsigned char* srcTex, int size, int imgWidth, int verticalOffset, int horizontalOffset, bool verticalFlip, bool horizontalFlip, int numComponents)
{
    /*  Copy from srcTex to destTex and flip if needed */
    if (destTex == nullptr || srcTex == nullptr) {
        throw std::invalid_argument("Null pointer provided for textures");
    }
    for (int row = 0; row < size; ++row) {
        for (int col = 0; col < size; ++col) {
            int srcRow = verticalFlip ? (size - 1 - row) : row;
            int srcCol = horizontalFlip ? (size - 1 - col) : col;

            int destIndex = (row * size + col) * numComponents;
            int srcIndex = ((srcRow + horizontalOffset) * imgWidth + (srcCol + verticalOffset)) * numComponents;

            memcpy(&destTex[destIndex], &srcTex[srcIndex], numComponents);
        }
    }
}

/******************************************************************************/
/*!
\fn     void Bump2Normal(   const unsigned char *bumpImg,
unsigned char *normalImg,
int width, int height)
\brief
Compute normal map from bump map.
\param  bumpImg
Given 1D bump map.
\param  normalImg
3D normal map to be computed.
\param  width
Width of the texture map.
\param  height
Height of the texture map.
*/
/******************************************************************************/

void Rendering::ResourceManager::Bump2Normal(const unsigned char* bumpImg, unsigned char* normalImg, int width, int height)
{
    float a = 40.0f;
    float scale = 1.0f / 255.0f; //normalize bumps from [0,255] to [0,1] for the normal computation

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {

            // Calculate indices with border check
            int index_x0 = x - 1 < 0 ? x : x - 1;
            int index_x2 = x + 1 >= width ? x : x + 1;
            int index_y0 = y - 1 < 0 ? y : y - 1;
            int index_y2 = y + 1 >= height ? y : y + 1;

            // Get the surrounding bump values and compute the tangents
            float bu = (bumpImg[y * width + index_x2] - bumpImg[y * width + index_x0]) * scale * (index_x0 == index_x2 ? a : 0.5f * a);
            float bv = (bumpImg[index_y2 * width + x] - bumpImg[index_y0 * width + x]) * scale * (index_y0 == index_y2 ? a : 0.5f * a);

            // Compute the normal
            Vec3 normal{ -bu,-bv,1.f };
            normal = Normalize(normal);

            // Map to RGB space
            normalImg[(y * width + x) * 3 + 0] = static_cast<unsigned char>(std::max(0, std::min(255, static_cast<int>((normal.x + 1.0f) * 127.5f))));
            normalImg[(y * width + x) * 3 + 1] = static_cast<unsigned char>(std::max(0, std::min(255, static_cast<int>((normal.y + 1.0f) * 127.5f))));
            normalImg[(y * width + x) * 3 + 2] = static_cast<unsigned char>(std::max(0, std::min(255, static_cast<int>((normal.z + 1.0f) * 127.5f))));
        }
    }
}
