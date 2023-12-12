#pragma once
#include "Mesh.h"
#include <math/Math.h>
#include <utilities/ToUnderlyingEnum.h>
#include <rendering/ResourceManager.h>
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
	//const int NUM_LIGHTS = 1;


	/*  Light pos are defined in world frame, but we need to compute their pos in view frame for
		lighting. In this frame, the vertices are not too scattered, hence the computation
		is normally more accurate.
	*/
	//extern Vec3 lightPosWF[NUM_LIGHTS], lightPosVF[NUM_LIGHTS];


	/*  Pre-defined light & material properties.
		We assume all objects in the scene have the same material.
	*/
	//const Vec4 I = { 0.6f, 0.6f, 0.6f, 1.0f };     /*  intensity of each light source */
	//const Vec4 ambientAlbedo = { 0.6f, 0.6f, 0.6f, 1.0f };     /*  ambient albedo of material */
	//const Vec4 diffuseAlbedo = { 0.6f, 0.6f, 0.6f, 1.0f };     /*  diffuse albedo of material */

	//const Vec4 specularAlbedo = { 1.0f, 1.0f, 1.0f, 1.0f };     /*  specular albedo of material */
	//const int  specularPower = 10;                              /*  specular power of material */


	///*  Mirror and sphere positions, which are used in graphics.cpp for rendering scene from these objects */
	//extern Vec3 mirrorTranslate;
	//extern Vec3 mirrorRotationAxis;
	//extern float mirrorRotationAngle;
	//extern Vec3 spherePos;


	/*  Set up all objects in the scene */
	//void SetUpScene();

	//--------------------

	//(temp) extern for now
	//extern std::vector<Object> obj;
	//extern Mesh mesh[static_cast<int>(MeshID::NUM_MESHES)];

	class Object {
	private:
		Mesh& m_mesh;     
		ImageID m_imageID;
		Mat4 m_modelMatrix;   

	public:
		Object::Object(Mesh& mesh, ImageID imageID, const Mat4& modelMat)
			: m_mesh(mesh), m_imageID(imageID), m_modelMatrix(modelMat) {
		}

		Object(const Object& other) = default;
		Object(Object&& other) noexcept = default;
		Object& operator=(const Object& other) = default;
		Object& operator=(Object&& other) noexcept = default;

		// Getter methods (setters might not be necessary because we are passing by reference)
		const Mesh& GetMesh() const { return m_mesh; }
		const Mat4& GetModelMatrix() const { return m_modelMatrix; }

		ImageID GetImageID() const { return m_imageID; }
		void SetImageID(ImageID id) { m_imageID = id; }
	};

}
