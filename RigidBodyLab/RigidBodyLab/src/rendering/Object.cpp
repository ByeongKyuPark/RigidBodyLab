#include <rendering/Object.h>

/*  Pre-defined meshes */
Mesh mesh[MeshID::NUM_MESHES] =
{
    CreateCube(1, 1, 1),
    LoadOBJMesh("../RigidBodyLab/models/vase.obj"),
    CreatePlane(1, 1),
    CreateSphere(16, 16)
};


/*  Pre-defined objects */
std::vector<Object> obj;

/*  Mirror and sphere positions, which are used in graphics.cpp for rendering scene from these objects */
Vec3 mirrorTranslate;
Vec3 mirrorRotationAxis;
float mirrorRotationAngle;
Vec3 spherePos;


/*  Light pos are defined in world frame, but we need to compute their pos in view frame for
    lighting. In this frame, the vertex positions are not too large, hence the computation
    is normally more accurate.
*/
Vec3 lightPosWF[NUM_LIGHTS], lightPosVF[NUM_LIGHTS];


/*  Setting up the light position */
void SetUpLight(float height);


/******************************************************************************/
/*!
\fn     void SetUpScene()
\brief
        Set up all objects in the scene.
*/
/******************************************************************************/
void SetUpScene()
{
    constexpr float BASE_POS_Y = -4.5f;
    constexpr float BASE_SCL_Y = 7.5f;
    constexpr float MIRROR_POS_Y = 2.4f;
    constexpr float MIRROR_SCL = 6.f;

    Vec3 baseSize = Vec3(11.0f, BASE_SCL_Y, 7.0f);
    Object base(MeshID::CUBE, ImageID::STONE_TEX, Translate(0,BASE_POS_Y,0)*Scale(baseSize));
    obj.push_back(base);
    
    Object vase(MeshID::VASE, ImageID::POTTERY_TEX, Translate(1.0f, -0.65f, 2.0f));
    obj.push_back(vase);

    mirrorTranslate = Vec3(1.0f, MIRROR_POS_Y, -1.5f);
    mirrorRotationAxis = BASIS[Y];
    mirrorRotationAngle = -EIGHTH_PI;
    /*  The 180-degree rotation about the y-axis is to simulate the mirroring effect.
        Also because of this the mirror normal vectors are pointing backwards and we
        have to switch the culled face to GL_BACK in graphics.cpp
    */                                                                                                                          //flipped the mirror
    Object mirror(MeshID::PLANE, ImageID::MIRROR_TEX, Translate(mirrorTranslate) * Scale(MIRROR_SCL, MIRROR_SCL, MIRROR_SCL)* Rotate(PI+mirrorRotationAngle, mirrorRotationAxis));
    obj.push_back(mirror);

    /*  These 3 parts are for the base of the mirror */
    constexpr float MIRROR_FRAME_OFFSET = 0.45f;
    Object mirrorBase1(MeshID::CUBE, ImageID::WOOD_TEX, Translate(mirrorTranslate + Vec3(0, 0, -0.03f)) *Rotate(mirrorRotationAngle, mirrorRotationAxis) * Scale(MIRROR_SCL+MIRROR_FRAME_OFFSET, MIRROR_SCL+ MIRROR_FRAME_OFFSET, 0.05f));
    obj.push_back(mirrorBase1);

    Object mirrorBase2(MeshID::CUBE, ImageID::WOOD_TEX, Translate(mirrorTranslate + Vec3(0, -3.1, -0.6f)) * Rotate(mirrorRotationAngle, mirrorRotationAxis) * Scale(3.0f, 0.1f, 1.0f));
    obj.push_back(mirrorBase2);

    Object mirrorBase3(MeshID::CUBE, ImageID::WOOD_TEX, Translate(mirrorTranslate + Vec3(0, -2.1, -0.53f)) * Rotate(mirrorRotationAngle, mirrorRotationAxis) * Rotate(TWO_PI / 3, BASIS[X]) * Scale(0.5f, 0.1f, 1.7f));
    obj.push_back(mirrorBase3);
    
    constexpr float SPHERE_RAD = 4.5f;
    spherePos = Vec3(-4.5f, BASE_POS_Y+BASE_SCL_Y/2.f+SPHERE_RAD/2.f, -1.5f);
    Object sphere(MeshID::SPHERE, ImageID::SPHERE_TEX, Translate(spherePos)* Scale(SPHERE_RAD,SPHERE_RAD,SPHERE_RAD));
    obj.push_back(sphere);


    SetUpLight(baseSize.x);
}


/******************************************************************************/
/*!
\fn     void SetUpLight(float height)
\brief
        Set up the light at (0, height, 0).
*/
/******************************************************************************/
void SetUpLight(float height)
{
    lightPosWF[0] = Vec3(0, height, 0);
}