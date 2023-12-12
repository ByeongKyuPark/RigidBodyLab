#pragma once
#include "Mesh.h"
#include <math/Math.h>
#include <vector>

/*  Object format, contains mesh data, texture, transformation matrices */
//struct Object
//{
//    int meshID;
//    int imageID;        /*  ID for texture */
//    Mat4 modelMat;        /*  model-to-world transformation */
//
//    Object(int meshID, int imageID,
//        Mat4 modelMat = Mat4()) :
//        meshID(meshID), imageID(imageID), modelMat(modelMat) { }
//};


///*  Pre-defined shapes: big flat cube, horizontal cube, vertical cube, sphere */
//struct MeshID
//{
//    enum { CUBE = 0, VASE, PLANE, SPHERE, NUM_MESHES };
//};


//extern Mesh mesh[MeshID::NUM_MESHES];


///*  Types of objects */
//struct ObjID
//{
//    enum { BASE = 0, VASE, MIRROR, MIRRORBASE1, MIRRORBASE2, MIRRORBASE3, SPHERE, NUM_OBJS };
//};


/*  Pre-defined objects */
//extern std::vector<Object> obj;


/*  The ID for texture loading */
//struct ImageID
//{
//    enum { STONE_TEX = 0, WOOD_TEX, POTTERY_TEX, MIRROR_TEX, SPHERE_TEX, NUM_IMAGES };
//};
namespace Rendering {

	/*  Pre-defined lights */
	const int NUM_LIGHTS = 1;


	/*  Light pos are defined in world frame, but we need to compute their pos in view frame for
		lighting. In this frame, the vertices are not too scattered, hence the computation
		is normally more accurate.
	*/
	extern Vec3 lightPosWF[NUM_LIGHTS], lightPosVF[NUM_LIGHTS];


	/*  Pre-defined light & material properties.
		We assume all objects in the scene have the same material.
	*/
	const Vec4 I = { 0.6f, 0.6f, 0.6f, 1.0f };     /*  intensity of each light source */
	const Vec4 ambientAlbedo = { 0.6f, 0.6f, 0.6f, 1.0f };     /*  ambient albedo of material */
	const Vec4 diffuseAlbedo = { 0.6f, 0.6f, 0.6f, 1.0f };     /*  diffuse albedo of material */

	const Vec4 specularAlbedo = { 1.0f, 1.0f, 1.0f, 1.0f };     /*  specular albedo of material */
	const int  specularPower = 10;                              /*  specular power of material */


	/*  Mirror and sphere positions, which are used in graphics.cpp for rendering scene from these objects */
	extern Vec3 mirrorTranslate;
	extern Vec3 mirrorRotationAxis;
	extern float mirrorRotationAngle;
	extern Vec3 spherePos;


	/*  Set up all objects in the scene */
	void SetUpScene();

	//--------------------

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

	//(temp) extern for now
	extern std::vector<Object> obj;
	extern Mesh mesh[static_cast<int>(MeshID::NUM_MESHES)];

	class Object {
	private:
		MeshID m_meshID;               // ID for the mesh
		ImageID m_imageID;             // ID for texture
		Mat4 m_modelMatrix;            // model-to-world transformation matrix

	public:
		// Constructor
		Object(MeshID meshID, ImageID imageID, const Mat4& modelMat = Mat4()) :
			m_meshID(meshID), m_imageID(imageID), m_modelMatrix(modelMat) {}

		// Copy and Move semantics (explicitly specified for clarification)
		Object(const Object& other) = default;
		Object(Object&& other) noexcept = default;
		Object& operator=(const Object& other) = default;
		Object& operator=(Object&& other) noexcept = default;

		// Getter and setter methods
		MeshID GetMeshID() const { return m_meshID; }
		void SetMeshID(MeshID id) { m_meshID = id; }

		ImageID GetImageID() const { return m_imageID; }
		void SetImageID(ImageID id) { m_imageID = id; }

		const Mat4& GetModelMatrix() const { return m_modelMatrix; }
		void SetModelMatrix(const Mat4& matrix) { m_modelMatrix = matrix; }

		//// Transformation methods
		//void Translate(const Vec3& translation) {
		//    m_ModelMatrix.Translate(translation);
		//}

		//void Rotate(float angle, const Vec3& axis) {
		//    m_ModelMatrix.Rotate(angle, axis);
		//}

		//void Scale(const Vec3& scaleFactor) {
		//    m_ModelMatrix.Scale(scaleFactor);
		//}

		// Other methods as needed...
		// For example, rendering the object, updating it, etc.
	};

}
