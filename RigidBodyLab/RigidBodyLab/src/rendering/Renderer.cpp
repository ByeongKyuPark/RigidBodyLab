#ifdef _MSC_VER
#pragma warning(push, 0)  // Microsoft Visual Studio
#define _CRT_SECURE_NO_WARNINGS
#endif

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include <glad/glad.h>
#include <rendering/Renderer.h>
#include <rendering/Mesh.h>
#include <rendering/Camera.h>
#include <core/Scene.h>
#include <rendering/ResourceManager.h>
#include <physics/Collider.h>
#include <input/input.h>
#include <math/Math.h>
#include <utilities/ToUnderlyingEnum.h>
#include <image_io.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <ctime>

#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

using Rendering::Renderer;
using Core::Object;
using Core::Scene;


/******************************************************************************/
/*!
\fn     void SetUpSkyBoxUniformLocations(GLuint prog)
\brief
        Look up the locations of uniform variables in the skybox shader program.
\param  prog
        The given skybox shader program ID.
*/
/******************************************************************************/
void Renderer::SetUpSkyBoxUniformLocations()
{
    GLuint prog = m_shaders[TO_INT(ProgType::SKYBOX_PROG)].GetProgramID();
    m_skyboxViewMatLoc = glGetUniformLocation(prog, "viewMat");
    m_skyboxTexCubeLoc = glGetUniformLocation(prog, "texCube");
}

/******************************************************************************/
/*!
\fn     void SetUpMainUniformLocations(GLuint prog)
\brief
        Look up the locations of uniform variables in the main shader program.
\param  prog
        The given shader program ID.
*/
/******************************************************************************/
//void Renderer::SetUpForwardUniformLocations()
//{
//    GLuint prog = m_shaders[TO_INT(ProgType::FORWARD_PROG)].GetProgramID();
//    m_mainMVMatLoc = glGetUniformLocation(prog, "mvMat");
//    m_mainNMVMatLoc = glGetUniformLocation(prog, "nmvMat");
//    m_mainProjMatLoc = glGetUniformLocation(prog, "projMat");
//
//    m_textureLoc = glGetUniformLocation(prog, "colorTex");
//
//    m_numLightsLoc = glGetUniformLocation(prog, "numLights");
//    m_lightOnLoc = glGetUniformLocation(prog, "lightOn");
//
//    m_ambientLoc = glGetUniformLocation(prog, "ambient");
//    m_specularPowerLoc = glGetUniformLocation(prog, "specularPower");
//
//    m_bumpTexLoc = glGetUniformLocation(prog, "bumpTex");
//    m_normalTexLoc = glGetUniformLocation(prog, "normalTex");
//    m_normalMappingOnLoc = glGetUniformLocation(prog, "normalMappingOn");
//    m_parallaxMappingOnLoc = glGetUniformLocation(prog, "parallaxMappingOn");
//
//    for (int i = 0; i < NUM_MAX_LIGHTS; ++i) {
//        std::string index = std::to_string(i);
//
//        m_lightPosLoc[i] = glGetUniformLocation(prog, ("lightPosVF[" + index + "]").c_str());
//        m_diffuseLoc[i] = glGetUniformLocation(prog, ("diffuse[" + index + "]").c_str());
//        m_specularLoc[i] = glGetUniformLocation(prog, ("specular[" + index + "]").c_str());
//    }
//
//}


/******************************************************************************/
/*!
\fn     void SetUpSphereUniformLocations(GLuint prog)
\brief
        Look up the locations of uniform variables in the sphere shader program.
\param  prog
        The given sphere shader program ID.
*/
/******************************************************************************/
void Renderer::SetUpSphereUniformLocations()
{
    GLuint prog = m_shaders[TO_INT(ProgType::SPHERE_PROG)].GetProgramID();
    m_sphereMVMatLoc = glGetUniformLocation(prog, "mvMat");
    m_sphereNMVMatLoc = glGetUniformLocation(prog, "nmvMat");
    m_sphereProjMatLoc = glGetUniformLocation(prog, "projMat");
    m_sphereViewMatLoc = glGetUniformLocation(prog, "viewMat");

    m_sphereTexCubeLoc = glGetUniformLocation(prog, "texCube");
    m_sphereRefLoc = glGetUniformLocation(prog, "sphereRef");
    m_sphereRefIndexLoc=glGetUniformLocation(prog, "sphereRefIndex");
}


/******************************************************************************/
/*!
\fn     void SetUpVertexData(Mesh &mesh)
\brief
        Create VAO (graphics state), VBO (vertex data state) and
        IBO (vertex index state) for the mesh, and feed vertex data into
        the shaders.
\param  mesh
        The input mesh, whose VAO/VBO/IBO members will be updated and data
        will be sent to shaders.
*/
/******************************************************************************/
void Renderer::SetUpVertexData(Mesh& mesh)
{ 
    glGenVertexArrays(1, &mesh.VAO);
    glBindVertexArray(mesh.VAO);

    glGenBuffers(1, &mesh.VBO);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.VBO);
    /*  Copy vertex attributes to GPU */
    glBufferData(GL_ARRAY_BUFFER,
        mesh.numVertices * vertexSize, &mesh.vertexBuffer[0],
        GL_STATIC_DRAW);

    glGenBuffers(1, &mesh.IBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.IBO);
    /*  Copy vertex indices to GPU */
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
        mesh.numIndices * indexSize, &mesh.indexBuffer[0],
        GL_STATIC_DRAW);

    /*  Send vertex attributes to shaders */
    for (int i = 0; i < numAttribs; ++i)
    {
        glEnableVertexAttribArray(vLayout[i].location);
        glVertexAttribPointer(vLayout[i].location, vLayout[i].size, vLayout[i].type,
            vLayout[i].normalized, vertexSize, (void*)vLayout[i].offset);
    }
}

void Rendering::Renderer::SetUpShaders() {
    for (const auto& pair : m_shaderFileMap) {
        auto& shader = m_shaders[TO_INT(pair.first)];
        shader.LoadShader(pair.second.vertexShaderPath, pair.second.fragmentShaderPath);
    }
    // For SKYBOX_PROG
    m_shaders[TO_INT(ProgType::SKYBOX_PROG)].Use();
    SetUpSkyBoxUniformLocations();

     //For SPHERE_PROG
    m_shaders[TO_INT(ProgType::SPHERE_PROG)].Use();
    SetUpSphereUniformLocations();

    // FORWARD_PROG
    //m_shaders[static_cast<int>(ProgType::FORWARD_PROG)].Use();
    //SetUpForwardUniformLocations();

    // (1) DEFERRED_GEOM 
	m_shaders[TO_INT(ProgType::DEFERRED_GEOMPASS)].Use();
	SetUpDeferredGeomUniformLocations();

    // (2) DEFERRED_LIGHT
	m_shaders[TO_INT(ProgType::DEFERRED_LIGHTPASS)].Use();
	SetUpDeferredLightUniformLocations();
}


/******************************************************************************/
/*!
\fn     void SetUpGTextures()
\brief
Set up the buffers for the outputs of geometry pass, which will then be
used for lighting computation in light pass.
*/
/******************************************************************************/
void Rendering::Renderer::SetUpGTextures()
{
    /*  Set up 16-bit floating-point, 4-component texture for color output */
     // Albedo (Color)
    constexpr int OFFSET = 20;
    glActiveTexture(GL_TEXTURE0+OFFSET);
    glGenTextures(1, &m_gColorTexID);
    glBindTexture(GL_TEXTURE_2D, m_gColorTexID);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA16F, Camera::DISPLAY_SIZE, Camera::DISPLAY_SIZE);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);

    /*  Set up 32-bit floating-point, 3-component texture for position output.
    Using 32 bits instead of 16 bits coz position may vary more widely and require
    higher accuracy.
    */
    glActiveTexture(GL_TEXTURE1 + OFFSET);
    glGenTextures(1, &m_gPosTexID);
    glBindTexture(GL_TEXTURE_2D, m_gPosTexID);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB32F, Camera::DISPLAY_SIZE, Camera::DISPLAY_SIZE);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);

    /*  Set up 16-bit floating-point, 3-component texture for normal output */
    glActiveTexture(GL_TEXTURE2 + OFFSET);
    glGenTextures(1, &m_gNrmTexID);
    glBindTexture(GL_TEXTURE_2D, m_gNrmTexID);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA16F, Camera::DISPLAY_SIZE, Camera::DISPLAY_SIZE);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);

    /*  Set up 32-bit floating-point texture for depth component output */
    glActiveTexture(GL_TEXTURE3 + OFFSET);
    glGenTextures(1, &m_gDepthTexID);
    glBindTexture(GL_TEXTURE_2D, m_gDepthTexID);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT32F, Camera::DISPLAY_SIZE, Camera::DISPLAY_SIZE);

    /*  Generate offscreen framebuffer ID and store it in gFrameBufferID.
    For deferred shading, this framebuffer is bounded to store
    the outputs.
    */
    glGenFramebuffers(1, &m_gFrameBufferID);

    glBindFramebuffer(GL_FRAMEBUFFER, m_gFrameBufferID);

    /*  Indicating the attachments to be used for the output buffers.
    Note that the ID of the attachment (e.g. COLOR_ATTACHMENT0) should match
    the layout location of the output in fragment shader.
    */
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_gColorTexID, 0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, m_gPosTexID, 0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, m_gNrmTexID, 0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_gDepthTexID, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "Framebuffer not complete!" << std::endl;
        exit(-1);
    }
}

/******************************************************************************/
/*!
\fn     void SetUpLightPassQuads()
\brief
Create VAO (graphics state) and VBO (vertex data state) for the quads
in deferred light pass. These quads will be used for full-screen render
and minimaps render.
*/
/******************************************************************************/
void Rendering::Renderer::SetUpLightPassQuads()
{
    for (int i = 0; i < TO_INT(DebugType::NUM_DEBUGTYPES); ++i)
    {
        glGenVertexArrays(1, &(quadVAO[i]));
        glBindVertexArray(quadVAO[i]);

        /*  Create vertex buffer ID to store the state of the quad vertices */
        glGenBuffers(1, &(quadVBO[i]));
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO[i]);

        /*  Allocate buffer for the quad vertex coordinates */
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadBuff[i]), quadBuff[i], GL_STATIC_DRAW);

        /*  Feed the quad vertex buffer into shaders */
        glEnableVertexAttribArray(m_lLightPassQuadLoc);
        glVertexAttribPointer(
            m_lLightPassQuadLoc,   // location of quadCoord variable in shader
            2,                  // dimension of vertex coord
            GL_FLOAT,           // type
            GL_FALSE,           // normalized or not
            0,                  // stride
            (void*)0            // array buffer offset
        );
    }
}

bool Rendering::Renderer::ShouldUpdateSphereCubemap(float speedSqrd) {
    if (m_shouldUpdateCubeMapForSphere) {
        return true;
    }
    constexpr float MIN_SPEED_SQRD = 0.001f;
    if (speedSqrd <= MIN_SPEED_SQRD) {
        return false;
    }

    constexpr int UPDATE_INTERVAL = 5;
    m_sphereMirrorCubeMapFrameCounter++;
    if (m_sphereMirrorCubeMapFrameCounter >= UPDATE_INTERVAL) {
        m_sphereMirrorCubeMapFrameCounter = 0;
        return true;
    }
    return false;
}

/******************************************************************************/
/*!
\fn     void SendLightProperties()
\brief
        Send numLights and intensities to the rendering program.
*/
/******************************************************************************/
//void Renderer::SendForwardProperties(const Core::Scene& scene)
//{
//    const int numLights = scene.GetNumLights();
//    glUniform1i(m_numLightsLoc, numLights);
//
//    /*  ambient, diffuse, specular are now reflected components on the object
//        surface and can be used directly as intensities in the lighting equation.
//    */
//    Vec4 globalAmbient = scene.m_ambientLightIntensity * scene.m_ambientAlbedo;
//    glUniform4fv(m_ambientLoc, 1, ValuePtr(globalAmbient));
//
//    for (int i = 0; i < numLights; ++i) {
//        Vec4 diffuse = scene.m_I[i] * scene.m_diffuseAlbedo;
//        Vec4 specular = scene.m_I[i] * scene.m_specularAlbedo;
//
//        glUniform4fv(m_diffuseLoc[i], 1, ValuePtr(diffuse));
//        glUniform4fv(m_specularLoc[i], 1, ValuePtr(specular));
//    }
//
//    glUniform1i(m_specularPowerLoc, scene.m_specularPower);
//}

void Rendering::Renderer::SendDeferredGeomProperties(const Scene& scene) {
    const int numLights = scene.GetNumLights();
    glUniform1i(m_gNumLightsLoc, numLights);
    glUniform1i(m_gNormalMappingOnLoc, m_parallaxMappingOn);
    glUniform1i(m_gParallaxMappingOnLoc, m_parallaxMappingOn);

    for (int i = 0; i < numLights; ++i) {
        glUniform4fv(m_gLightPosVFLoc[i], 1, ValuePtr(scene.m_lightPosVF[i]));
    }

    Vec4 ambient = scene.m_ambientLightIntensity * scene.m_ambientAlbedo;
    Vec4 diffuse, specular;

    /*  Send ambient, diffuse, specular, specularPower, and blinnPhongLighting(boolean) to shader. */
    glUniform4fv(m_gAmbientLoc, 1, &ambient[0]);
    glUniform1i(m_gSpecularPowerLoc, scene.m_specularPower);

    for (int i = 0; i < numLights; ++i) {
        diffuse = scene.m_I[i] * scene.m_diffuseAlbedo;
        specular = scene.m_I[i] * scene.m_specularAlbedo;

        glUniform4fv(m_gDiffuseLoc[i], 1, &diffuse[i]);
        glUniform4fv(m_gSpecularLoc[i], 1, &specular[i]);
        glUniform4fv(m_gLightPosVFLoc[i], 1, ValuePtr(scene.m_lightPosVF[i]));
    }
}

void Rendering::Renderer::SendDeferredLightPassProperties(const Scene& scene)
{
    const int numLights = scene.GetNumLights();
    glUniform1i(m_lNumLightsLoc, numLights);
    glUniform1i(m_lLightPassDebugLoc, m_gLightPassDebug);
    glUniform1i(m_lParallaxMappingOnLoc, m_parallaxMappingOn);
    glUniform1i(m_lBlinnPhongLightingLoc, m_gBlinnPhongLighting);
    glUniform1i(m_lNormalMappingObjTypeLoc, TO_INT(Core::ObjectType::NORMAL_MAPPED_PLANE));

    /*  ambient, diffuse, specular are now reflected components on the object
        surface and can be used directly as intensities in the lighting equation.
    */
    Vec4 ambient, diffuse, specular;
    
    ambient = scene.m_ambientLightIntensity * scene.m_ambientAlbedo;

    /*  Send ambient, diffuse, specular, specularPower, and blinnPhongLighting(boolean) to shader. */
    glUniform4fv(m_lAmbientLoc, 1, &ambient[0]);
    glUniform1i(m_lSpecularPowerLoc, scene.m_specularPower);

    for (int i = 0; i < numLights; ++i) {
        diffuse = scene.m_I[i] * scene.m_diffuseAlbedo;
        specular = scene.m_I[i] * scene.m_specularAlbedo;

		glUniform4fv(m_lDiffuseLoc[i], 1, &diffuse[i]);
		glUniform4fv(m_lSpecularLoc[i], 1, &specular[i]);
        glUniform4fv(m_lLightPosVFLoc[i], 1, ValuePtr(scene.m_lightPosVF[i]));
    }
}


/******************************************************************************/
/*!
\fn     ComputeObjMVMats(Mat4 MVMat[], Mat4 NMVMat[], const Mat4& viewMat)
\brief
        Compute the modelview matrices for positions and normals.
\param  MVMat
        Modelview matrix for positions to be computed.
\param  NMVMat
        Modelview matrix for normals to be computed.
\param  viewMat
        Given view matrix.
*/
/******************************************************************************/
void Renderer::ComputeMainCamObjMVMats(const Core::Scene& scene)
{
    const size_t objSize = scene.m_objects.size();
    for (int i = 0; i < objSize; ++i)
    {
        Mat4 objMat = scene.m_objects[i]->GetModelMatrix();
        m_mainCamMVMat[i] = m_mainCamViewMat * objMat;
        m_mainCamNormalMVMat[i] = Transpose(Inverse(m_mainCamMVMat[i]));
    }
}

void Renderer::ComputePlanarMirrorCamObjMVMats(const Core::Scene& scene)
{
    const size_t objSize = scene.m_objects.size();
    for (int i = 0; i < objSize; ++i)
    {
        Mat4 objMat = scene.m_objects[i]->GetModelMatrix();
        m_mirrorCamMVMat[i] = m_mirrorCamViewMat * objMat;
        m_mirrorCamNormalMVMat[i] = Transpose(Inverse(m_mirrorCamMVMat[i]));
    }
}

void Renderer::ComputeSphericalMirrorCamObjMVMats(int faceIdx,const Core::Scene& scene)
{
    const size_t objSize = scene.m_objects.size();
    for (int i = 0; i < objSize; ++i)
    {
        Mat4 objMat = scene.m_objects[i]->GetModelMatrix();
        m_sphereCamMVMat[i][faceIdx] = m_sphereCamViewMat[faceIdx] * objMat;
        m_sphereCamNormalMVMat[i][faceIdx] = Transpose(Inverse(m_sphereCamMVMat[i][faceIdx]));
    }
}


/******************************************************************************/
/*!
\fn     void ComputeMainCamMats()
\brief
        Compute the view/projection and other related matrices for user camera.
*/
/******************************************************************************/
void Renderer::ComputeMainCamMats(const Core::Scene& scene)
{
    /*  Update projection matrix */
    if (mainCam.resized) {
        m_mainCamProjMat = mainCam.ProjMat();
    }

    /*  Update view transform matrix */
    if (mainCam.moved || mirrorCam.moved){
        m_mainCamViewMat = mainCam.ViewMat();
        ComputeMainCamObjMVMats(scene);
    }

}


/******************************************************************************/
/*!
\fn     void ComputeMirrorCamMats(const Core::Scene& scene)
\brief
        Compute the view/projection and other related matrices for mirror camera.
*/
/******************************************************************************/
void Renderer::ComputeMirrorCamMats(const Core::Scene& scene)
{
    if (scene.m_mirror == nullptr) {
        return;
    }

	//check if mirror has moved (check only the translation not rotation for now)
	// Thresholds for movement
	static constexpr float POSITION_THRESHOLD = 0.01f;

	static Math::Vector3 previousMirrorPos = scene.m_mirror->GetPosition();
	const Math::Vector3& currentMirrorPos = previousMirrorPos;

    Math::Vector3 positionDelta = currentMirrorPos - previousMirrorPos;
	previousMirrorPos = currentMirrorPos;
	
    if (positionDelta.Length() > POSITION_THRESHOLD) {
		mirrorCam.moved = true;
	}

    if (mainCam.moved||mirrorCam.moved)
    {

        Mat4 mirrorMat=scene.m_mirror->GetModelMatrix();
        Vec3 mainCamMirrorFrame = Vec3(Inverse(mirrorMat) * Vec4(mainCam.pos, 1.0));

        /*  If user camera is behind mirror, then mirror is not visible and no need to compute anything */
        if (mainCamMirrorFrame.z >= 0)
        {
            m_mirrorVisible = false;
            return;
        }
        else {
            m_mirrorVisible = true;
        }

        /*  In mirror frame, mirror camera position is defined as (x, y, -z) in which (x, y, z) is the
            user camera position in mirror frame.
            We also need to compute mirrorCam.pos, mirrorCam.upVec, mirrorCam.lookAt are defined in world
            frame to compute mirror cam's view matrix.
            function to compute m_mirrorCamViewMat
        */

        Vec3 mirrorCamMirrorFrame = Vec3(mainCamMirrorFrame.x, mainCamMirrorFrame.y, -mainCamMirrorFrame.z);
        Vec3 mirrorNormal = Normalize(Vec3(mirrorMat * Vec4(0, 0, 1, 0)));

        // Setting mirror camera's position and look-at point
        mirrorCam.pos = Vec3(mirrorMat * Vec4(mirrorCamMirrorFrame, 1.0));
        mirrorCam.upVec = Normalize(Vec3(mirrorMat * Vec4(0, 1, 0, 0)));
        Vec3 mirrorCenter = scene.m_mirror->GetMesh()->m_boundingBox.center;
        mirrorCam.lookAt = Vec3(mirrorMat* Vec4{ mirrorCenter,1.f });

        m_mirrorCamViewMat = LookAt(mirrorCam.pos, mirrorCam.lookAt, mirrorCam.upVec);

        ComputePlanarMirrorCamObjMVMats(scene);
        /*  Compute mirror camera projection matrix */
        /*  In mirror frame, the mirror camera view direction is towards the center of the mirror,
            which is the origin of this frame.

            mirrorCam.nearPlane is computed as the smallest projected length along the view direction
            of the vectors from the mirror camera to the midpoints of the mirror left, right, bottom
            and top edges. These midpoints are pre-defined in mirror frame as (0.5, 0, 0) [left],
            (-0.5, 0, 0) [right], (0, -0.5, 0) [bottom] and (0, 0.5, 0) [top].

            mirrorCam.farPlane is set to INFINITY.

            leftPlane, rightPlane, bottomPlane, topPlane are computed based on the intersections between
            the near plane and the vectors from camera to the left, right, bottom and top edge midpoints.
            After you've done with this computation, you would see the reflected image with a quite narrow
            view angle (which is physically correct). However, in order to widen the view angle, we would
            multiply all leftPlane, rightPlane, bottomPlane, topPlane by 2.
        */
        Vec3 mirrorCamViewMirrorFrame = Normalize(mirrorCam.lookAt - mirrorCam.pos);

        //mirror frame
        std::vector<Vec3> midPoints = {
            Vec3(0.5, 0, 0), // Left
            Vec3(-0.5, 0, 0), // Right
            Vec3(0, -0.5, 0), // Bottom
            Vec3(0, 0.5, 0) // Top
        };


        float nearDist = INFINITY;
        for (Vec3& midPoint : midPoints) {
            Vec3 midCamFrame = Vec3(mirrorMat * Vec4(midPoint, 1.0));//mid : mirrorFrame -> cameraFrame
            midPoint = midCamFrame - mirrorCam.pos;//mid : cameraFrame -> mirroredCameraFrame
            float projectionLength = Dot(midPoint, mirrorCamViewMirrorFrame);
            nearDist = std::min(nearDist, projectionLength);
        }

        // Note that midsPoint are now in mirrorCameraFrame

        //(trivial)
        // Setting the far plane to infinity can lead to depth precision issues, causing distant objects to dominate in reflections. 
        // A minimum near plane distance is set to mitigate this, particularly suitable for this static scene scenarios.
        //mirrorCam.nearPlane = std::max(nearDist, 2.5f);
        constexpr float MAX_FAR_PLANE = 100.f;
        constexpr float MIN_NEAR_PLANE = 2.5f;

        mirrorCam.nearPlane = std::max(nearDist, MIN_NEAR_PLANE);
        mirrorCam.farPlane = MAX_FAR_PLANE;

        // Compute planes based on intersections
        Vec3 toNearMirrorCamFrame = mirrorCamViewMirrorFrame * nearDist;//simply 'k' in the note

        // Returns the intersection point on the near plane in the mirrored camera frame, scaled from the mirrored camera's position
        auto ComputeIntersectionOnNearPlane = [&](const Vec3& midpointMirrorCamFrame) -> Vec3 {
            float numerator = Dot(mirrorCamViewMirrorFrame, toNearMirrorCamFrame);
            float denominator = Dot(midpointMirrorCamFrame, mirrorCamViewMirrorFrame);
            float t = numerator / denominator;
            return mirrorCam.pos + t * midpointMirrorCamFrame;
        };

        Vec3 left = ComputeIntersectionOnNearPlane(midPoints[0]);
        Vec3 right = ComputeIntersectionOnNearPlane(midPoints[1]);
        Vec3 bottom = ComputeIntersectionOnNearPlane(midPoints[2]);
        Vec3 top = ComputeIntersectionOnNearPlane(midPoints[3]);

        mirrorCam.leftPlane = -(toNearMirrorCamFrame - left).length();
        mirrorCam.rightPlane = (toNearMirrorCamFrame - right).length();
        mirrorCam.bottomPlane = -(toNearMirrorCamFrame - bottom).length();
        mirrorCam.topPlane = (toNearMirrorCamFrame - top).length();

        //float viewAngleAdjustFactor = 1.f; //scales down the frustum planes
        //mirrorCam.leftPlane *= viewAngleAdjustFactor;
        //mirrorCam.rightPlane *= viewAngleAdjustFactor;
        //mirrorCam.bottomPlane *= viewAngleAdjustFactor;
        //mirrorCam.topPlane *= viewAngleAdjustFactor;
        m_mirrorCamProjMat = mirrorCam.ProjMat();
    }
}


/******************************************************************************/
/*!
\fn     void ComputeSphereCamMats()
\brief
        Compute the view/projection and other related matrices for sphere camera.
*/
/******************************************************************************/
void Renderer::ComputeSphereCamMats(const Core::Scene& scene)
{
    if (!scene.m_sphere) {
        return;
    }
    /*  Compute the lookAt positions for the 6 faces of the sphere cubemap.
        The sphere camera is at spherePos.
        The front and back faces are -z and +z.
        The left and right faces are -x and +x.
        The bottom and top faces are -y and +y.
    */
    // Directions for the cubemap faces
    const static Vec3 lookAt[TO_INT(CubeFaceID::NUM_FACES)] = {
        BASIS[0], // RIGHT
        -BASIS[0], // LEFT
        BASIS[1], // TOP
        -BASIS[1], // BOTTOM
        BASIS[2], // BACK
        -BASIS[2]  // FRONT
    };

    /*  Compute upVec for the 6 faces of the sphere cubemap.
        The front, back, left and right faces are flipped upside-down.
        The top face's upVec is pointing forward and the bottom face's
        upVec is pointing backward.
    */
    // Up vectors for the cubemap faces
    const static Vec3 upVec[TO_INT(CubeFaceID::NUM_FACES)] = {
        -BASIS[1], // RIGHT flipped
        -BASIS[1], // LEFT flipped
        BASIS[2],  // TOP flipped
        -BASIS[2], // BOTTOM flipped
        -BASIS[1], // BACK flipped
        -BASIS[1]  // FRONT flipped
    };

    for (int f = 0; f < TO_INT(CubeFaceID::NUM_FACES); ++f)
    {
        Vec3 spherePos = { scene.m_sphere->GetPosition().x,scene.m_sphere->GetPosition().y,scene.m_sphere->GetPosition().z };
        m_sphereCamViewMat[f] = LookAt(spherePos, spherePos + lookAt[f], upVec[f]);
        ComputeSphericalMirrorCamObjMVMats(f, scene);
    }


    /*  Use Perspective function to ompute the projection matrix m_sphereCamProjMat so that
        from the camera position at the cube center, we see a complete face of the cube.
        The near plane distance is 0.01f. The far plane distance is equal to mainCam's farPlane.
    */
    // Compute the projection matrix for the sphere camera
    constexpr float fov = PI / 2.f; // 90 degrees in radians
    constexpr float aspectRatio = 1.f;
    constexpr float nearPlane = 0.01f; // near plane is 0.01, and far plane is the same as the main camera's far plane
    m_sphereCamProjMat = Perspective(fov, aspectRatio, nearPlane, mainCam.farPlane);
}


void Rendering::Renderer::RenderGui(Scene& scene, float fps) {

    ImGui::SetNextWindowPos(ImVec2(Camera::DISPLAY_SIZE, 0));
    ImGui::SetNextWindowSize(ImVec2(Camera::GUI_WIDTH, Camera::GUI_WIDTH * 5.f));

    // displaying FPS
    ImGui::Text("Frame Rate: %.1f", fps);

    // sphere Reflection/Refraction settings
    int refTypeInt = static_cast<int>(m_sphereRef);
    const char* refTypes[] = { "Reflection Only", "Refraction Only", "Reflection & Refraction" };
    if (ImGui::Combo("Sphere", &refTypeInt, refTypes, IM_ARRAYSIZE(refTypes))) {
        m_sphereRef = static_cast<RefType>(refTypeInt);
    }
    if (m_sphereRef != RefType::REFLECTION_ONLY) {
        ImGui::SliderFloat("Sphere Refractive Index", &m_sphereRefIndex, 1.0f, 2.5f);
    }

    // parallax Mapping Toggle
    bool& parallaxMappingOn = Renderer::GetInstance().GetParallaxMapping();
    if(ImGui::Checkbox("Parallax Mapping", &parallaxMappingOn)) {
        glUniform1i(m_lParallaxMappingOnLoc, parallaxMappingOn);
    }

    ImGui::Checkbox("Display Deferred Rendering Textures", &m_buffersDisplay);

    // obj List GUI
    static int selectedObject = -1;
    std::vector<std::string> objectNames;
    for (size_t i = 0; i < scene.m_objects.size(); ++i) {
        if (scene.m_objects[i]->GetCollider()->GetCollisionEnabled() == true) {
            objectNames.emplace_back(scene.m_objects[i]->GetName());
        }
    }
    if (ImGui::ListBox("Objects", &selectedObject, [](void* data, int idx, const char** out_text) -> bool {
        const auto& vector = *static_cast<const std::vector<std::string>*>(data);
    if (idx < 0 || idx >= static_cast<int>(vector.size())) { return false; }
    *out_text = vector.at(idx).c_str();
    return true;
        }, static_cast<void*>(&objectNames), objectNames.size())) {
    }
    // mesh and texture Names Setup
    const static std::array<std::string,TO_INT(MeshID::NUM_MESHES)> meshNames = {"Cube", "Vase", "Plane", "Sphere", "Teapot", "Diamond", "Dodecahedron", "Gourd"};
    const static std::array<std::string, TO_INT(ImageID::NUM_IMAGES)> textureNames = { "Stone", "Stone2", "Wood1", "Wood2", "Pottery1", "Pottery2", "Pottery3" };

    // convert std::string vectors to const char* arrays for ImGui
    const char* meshNamesCStr[meshNames.size()];
    const char* textureNamesCStr[textureNames.size()];
    for (size_t i = 0; i < meshNames.size(); ++i) {
        meshNamesCStr[i] = meshNames[i].c_str();
    }
    for (size_t i = 0; i < textureNames.size(); ++i) {
        textureNamesCStr[i] = textureNames[i].c_str();
    }

    // Ensure selected object index is within valid range
    if (selectedObject >= 0 && selectedObject < static_cast<int>(objectNames.size())) {
        int sceneObjectIndex = m_guiToObjectIndexMap[selectedObject];
        std::string selectedObjectName = objectNames[selectedObject];

        static int selectedMesh{ -1 };
        static int selectedTexture{ -1 };

        // Mesh selection
        if (ImGui::Combo("Meshes", &selectedMesh, meshNamesCStr, meshNames.size())) {
            if (selectedMesh >= 0) {
                Mesh* newMesh = ResourceManager::GetInstance().GetMesh(static_cast<MeshID>(selectedMesh));
                scene.GetObject(sceneObjectIndex).SetMesh(newMesh);
                m_shouldUpdateCubeMapForSphere = true;
            }
        }

        // Texture selection
        if (selectedObjectName != "spherical mirror" && selectedObjectName != "planar mirror") {
            if (ImGui::Combo("Textures", &selectedTexture, textureNamesCStr, textureNames.size())) {
                scene.GetObject(sceneObjectIndex).SetImageID(static_cast<ImageID>(selectedTexture));
                m_shouldUpdateCubeMapForSphere = true;
            }
        }
    }
    else {
        ImGui::Text("No object selected");
    }

    // object Creation Section
    if (ImGui::CollapsingHeader("Add Object", ImGuiTreeNodeFlags_DefaultOpen)) {
        static char objectName[128] = "";
        static int meshID = 0;
        static int textureID = 0;
        static int colliderType = 0;
        static float radius = 1.0f;
        static float scale[3] = { 1.0f, 1.0f, 1.0f };
        static float position[3] = { 0.0f, 5.0f, 0.0f };
        static float mass = 1.0f;
        static float angleDegrees = 0.0f;
        static Math::Vector3 rotationAxis = Math::Vector3{ 0.0f, 1.0f, 0.0f };

        ImGui::InputText("Object Name", objectName, IM_ARRAYSIZE(objectName));
        ImGui::Combo("Mesh", &meshID, meshNamesCStr, meshNames.size());
        ImGui::Combo("Texture", &textureID, textureNamesCStr, textureNames.size());
        ImGui::RadioButton("Box Collider", &colliderType, 0); ImGui::SameLine();
        ImGui::RadioButton("Sphere Collider", &colliderType, 1);

        if (colliderType == 1) { ImGui::InputFloat("Radius", &radius); }
        else { ImGui::InputFloat3("Scale", scale); }

        ImGui::InputFloat3("Position", position);
        ImGui::InputFloat("Mass", &mass);
        ImGui::InputFloat("Rotation Angle (Degrees)", &angleDegrees);
        ImGui::InputFloat3("Rotation Axis", reinterpret_cast<float*>(&rotationAxis));

        if (ImGui::Button("Add")) {
            Core::ColliderConfig colliderConfig = (colliderType == 1) ?
                Core::ColliderConfig(radius) :
                Core::ColliderConfig(Vec3{ scale[0], scale[1], scale[2] });
            Math::Quaternion orientation(angleDegrees, rotationAxis);
            scene.CreateObject(
                objectName,
                static_cast<MeshID>(meshID),
                static_cast<ImageID>(textureID),
                static_cast<Physics::ColliderType>(colliderType),
                colliderConfig,
                { position[0], position[1], position[2] },
                mass,
                orientation
            );
            UpdateGuiToObjectIndexMap(scene);
        }
    }

    // projectile Launch Section
    if (ImGui::CollapsingHeader("Launch Projectile", ImGuiTreeNodeFlags_DefaultOpen)) {
        if (ImGui::Button("Shoot Projectile")) {
            // create and launch a projectile
            scene.ShootProjectile({ mainCam.GetPos().x,mainCam.GetPos().y,mainCam.GetPos().z });
        }
        UpdateGuiToObjectIndexMap(scene);
    }

    //lights
    if (ImGui::CollapsingHeader("Light Settings", ImGuiTreeNodeFlags_DefaultOpen)) {
        int numLights = scene.GetNumLights();

        if (ImGui::Button("Add Light") && numLights < NUM_MAX_LIGHTS) {
            UpdateNumLights(scene.AddLight());
            m_shouldUpdateCubeMapForSphere = true;
        }
        ImGui::SameLine();

        if (ImGui::Button("Remove Light") && numLights > 0) {
            UpdateNumLights(scene.RemoveLight());
            m_shouldUpdateCubeMapForSphere = true;
        }

        for (int i = 0; i < scene.GetNumLights(); ++i) {
            Vec3 lightPos = scene.GetLightPosition(i);
            bool posUpdated{ false }, colorUpdated{ false };
            if (posUpdated=ImGui::SliderFloat3(("Light " + std::to_string(i) + " Position").c_str(), &lightPos.x, -10.0f, 10.0f)) {
                scene.SetLightPosition(lightPos, i);
            }

            Vec4 lightColor = scene.GetLightColor(i);
            if (colorUpdated=ImGui::ColorEdit3(("Light " + std::to_string(i) + " Color").c_str(), &lightColor.x)) {
                scene.SetLightColor(lightColor, i);
            }
            if (posUpdated || colorUpdated) {
                SendLightProperties(scene);
            }
        }
    }
}

/******************************************************************************/
/*!
\fn     void SendMVMat(const Object &obj)
\brief
        Send object's model-view matrices to the shader.
\param  obj
        The object whose model-view matrices we want to send.
*/
/******************************************************************************/
void SendMVMat(const Mat4& mvMat, const Mat4& nmvMat, GLint mvMatLoc, GLint nmvMatLoc)
{
    /*  Send transformation matrices to shaders */
    glUniformMatrix4fv(mvMatLoc, 1, GL_FALSE, ValuePtr(mvMat));
    glUniformMatrix4fv(nmvMatLoc, 1, GL_FALSE, ValuePtr(nmvMat));
}


/******************************************************************************/
/*!
\fn     void SendViewMat(Mat4 viewMat, GLint viewMatLoc)
\brief
        Send view matrix to the corresponding variable.
        We send this separately because the skybox rendering only needs
        view matrix but not modeling or projection matrix.
\param  viewMat
        The view matrix to be sent.
\param  viewMatLoc
        The location of the variable to send to.
*/
/******************************************************************************/
void SendViewMat(Mat4 viewMat, GLint viewMatLoc)
{
    glUniformMatrix4fv(viewMatLoc, 1, GL_FALSE, ValuePtr(viewMat));
}


/******************************************************************************/
/*!
\fn     void SendProjMat()
\brief
        Send projection matrix to the shader.
\param  obj
        The object whose projection matrix we want to send.
*/
/******************************************************************************/
void SendProjMat(const Mat4& projMat, GLint projMatLoc)
{
    glUniformMatrix4fv(projMatLoc, 1, GL_FALSE, ValuePtr(projMat));
}

/******************************************************************************/
/*!
\fn     void SendCubeTexID(int texID, GLint texCubeLoc)
\brief
        Send cubemap texture ID to the corresponding variable.
\param  texID
        Texture ID to be sent.
\param  texCubeLoc
        Location of the variable to send to.
*/
/******************************************************************************/
void SendCubeTexID(int texID, GLint texCubeLoc)
{
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, texID);
    glUniform1i(texCubeLoc, 0);
}

GLFWwindow* Rendering::Renderer::GetWindow() const {
    return m_window.get();
}

/******************************************************************************/
/*!
\fn     void SendMirrorTexID()
\brief
        Send mirror texture ID to the corresponding variable.
*/
/******************************************************************************/
void Renderer::SendMirrorTexID()
{
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, ResourceManager::GetInstance().m_mirrorTexID);
    glUniform1i(m_gColorTexLoc, 0);
}

/******************************************************************************/
/*!
\fn     void SendObjTexID(GLuint texID, int activeTex)
\brief
        Send object's texture ID to the shader.
\param  texID
        The texture object we want to bind to.
\param  activeTex
        The active texture handler we want to use to map with the sampler variable.
\param  texLoc
        The location of the uniform sampler in the shader.
*/
/******************************************************************************/
void SendObjTexID(GLuint texID, int activeTex, GLint texLoc)
{
    glActiveTexture(GL_TEXTURE0 + activeTex);
    glBindTexture(GL_TEXTURE_2D, texID);
    glUniform1i(texLoc, activeTex);
}


/******************************************************************************/
/*!
\fn     void SetUp()
\brief
        Set up the render program and graphics-related data for rendering.
*/
/******************************************************************************/
void Renderer::AttachScene(const Core::Scene& scene)
{
    
    //1. shader
    SetUpShaders();

    //2. Send mesh data only
    ResourceManager& resourceManager = ResourceManager::GetInstance();
    size_t NUM_MESHES = TO_INT(MeshID::NUM_MESHES);
    for (int i = 0; i < NUM_MESHES; ++i) {
        SetUpVertexData(*resourceManager.GetMesh(static_cast<MeshID>(i)));
    }

    //3. obj textures
    resourceManager.SetUpTextures();

    //4. (deferred shading) Set up textures to be written to in geometry pass and read from in light pass
	SetUpGTextures();
	//5. (deferred shading) Set up full-screen quad for rendering deferred light pass and 4 small quads for debugging
	SetUpLightPassQuads();

    //m_shaders[TO_INT(ProgType::FORWARD_PROG)].Use();
    //SendForwardProperties(scene);

    m_shaders[TO_INT(ProgType::DEFERRED_LIGHTPASS)].Use();
    SendDeferredLightPassProperties(scene);

    m_shaders[TO_INT(ProgType::DEFERRED_GEOMPASS)].Use();
    SendDeferredGeomProperties(scene);

    /*  Drawing using filled mode */
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    /*  Hidden surface removal */
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    /*  Enable writing to depth buffer */
    glDepthMask(GL_TRUE);

    glEnable(GL_CULL_FACE);     /*  For efficiency, not drawing back-face */
    glCullFace(GL_BACK);
}


/******************************************************************************/
/*!
\fn     void CleanUp()
\brief
        Clean up the graphics-related stuffs before shutting down.
*/
/******************************************************************************/
void Renderer::CleanUp()
{
    // ImGui cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glBindVertexArray(0);

    ResourceManager& resourceManager = ResourceManager::GetInstance();
    size_t NUM_MESHES = TO_INT(MeshID::NUM_MESHES);
    for (int i = 0; i < NUM_MESHES; ++i)
    {
        Mesh* mesh = resourceManager.GetMesh(static_cast<MeshID>(i));
        glDeleteVertexArrays(1, &mesh->VAO);
        glDeleteBuffers(1, &mesh->VBO);
        glDeleteBuffers(1, &mesh->IBO);
    }

    glDeleteTextures(TO_INT(ImageID::NUM_IMAGES), resourceManager.m_textureIDs.data());
    glDeleteTextures(1, &resourceManager.m_bumpTexID);
    glDeleteTextures(1, &resourceManager.m_normalTexID);
    glDeleteTextures(1, &resourceManager.m_skyboxTexID);
    glDeleteTextures(1, &resourceManager.m_mirrorTexID);
    glDeleteTextures(1, &resourceManager.m_sphereTexID);

    glDeleteVertexArrays(TO_INT(DebugType::NUM_DEBUGTYPES), quadVAO);
    glDeleteBuffers(TO_INT(DebugType::NUM_DEBUGTYPES), quadVBO);

    glDeleteTextures(1, &m_gColorTexID);
    glDeleteTextures(1, &m_gPosTexID);
    glDeleteTextures(1, &m_gNrmTexID);
    glDeleteTextures(1, &m_gDepthTexID);

    glDeleteFramebuffers(1, &m_gFrameBufferID);
    glDeleteFramebuffers(1, &resourceManager.m_mirrorFrameBufferID);

}

Rendering::Renderer::Renderer()
    : m_window{ nullptr,WindowDeleter }, m_fps(0)
    , m_sphereRef(RefType::REFLECTION_ONLY)
    , m_parallaxMappingOn(true), m_sphereRefIndex{ 1.33f }//water by default
    , m_shouldUpdateCubeMapForSphere{ true }
    , m_sphereMirrorCubeMapFrameCounter{}

    , m_mainCamViewMat{}
    , m_mainCamProjMat{}
    , m_mainCamMVMat{}
    , m_mainCamNormalMVMat{}

    , m_mirrorCamViewMat{}
    , m_mirrorCamProjMat{}
    , m_mirrorCamMVMat{}
    , m_mirrorCamNormalMVMat{}

    , m_sphereCamProjMat{}
    , m_sphereCamViewMat(TO_INT(CubeFaceID::NUM_FACES))
    , m_gColorTexID {}
    , m_gPosTexID {}
    , m_gNrmTexID {}
    , m_gDepthTexID {}
    , m_gFrameBufferID {}
{
	// Initialize GLFW
	if (!glfwInit()) {
		std::cerr << "Failed to initialize GLFW\n";
		exit(EXIT_FAILURE);
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4); 
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // Required for MacOS
#endif

	// Create a windowed mode window and its OpenGL context
	GLFWwindow* rawWindow = glfwCreateWindow(Camera::DISPLAY_SIZE+Camera::GUI_WIDTH, Camera::DISPLAY_SIZE, "RigidBodyLab", nullptr, nullptr);
	if (!rawWindow) {
		glfwTerminate();
		std::cerr << "Failed to create GLFW window\n";
		exit(EXIT_FAILURE);
	}
	m_window.reset(rawWindow); // Assign rawWindow to m_window

	// Make the window's context current
	glfwMakeContextCurrent(m_window.get());

	// Set callback functions...
	glfwSetFramebufferSizeCallback(m_window.get(), Renderer::Resize);
	glfwSetKeyCallback(m_window.get(), Keyboard);
	glfwSetCursorPosCallback(m_window.get(), MouseMove);
	glfwSetScrollCallback(m_window.get(), MouseScroll);
	//Set the user pointer of the GLFW window to point to the Renderer (for the "Keyboard" function in input.cpp)
	glfwSetWindowUserPointer(m_window.get(), this);

	InitRendering();
	InitImGui();

    //m_shaderFileMap[ProgType::FORWARD_PROG] = { "../RigidBodyLab/shaders/main.vs",  "../RigidBodyLab/shaders/main.fs" };
    m_shaderFileMap[ProgType::SKYBOX_PROG] = { "../RigidBodyLab/shaders/skybox.vs", "../RigidBodyLab/shaders/skybox.fs" };
    m_shaderFileMap[ProgType::SPHERE_PROG] = { "../RigidBodyLab/shaders/sphere.vs", "../RigidBodyLab/shaders/sphere.fs" };
    //m_shaderFileMap[ProgType::DEFERRED_FORWARD] = { "../RigidBodyLab/shaders/deferred_forward.vs", "../RigidBodyLab/shaders/deferred_forward.fs" };
    m_shaderFileMap[ProgType::DEFERRED_GEOMPASS] = { "../RigidBodyLab/shaders/deferred_geom.vs", "../RigidBodyLab/shaders/deferred_geom.fs" };
    m_shaderFileMap[ProgType::DEFERRED_LIGHTPASS] = { "../RigidBodyLab/shaders/deferred_light.vs", "../RigidBodyLab/shaders/deferred_light.fs" };
}

Renderer& Rendering::Renderer::GetInstance() {
    static Renderer instance;
    return instance;
}

/******************************************************************************/
/*!
\fn     void Resize(GLFWwindow* window, int w, int h)
\brief
        Update viewport and projection matrix upon window resize.
\param  window
        glfw window
\param  w
        The new width of the window
\param  h
        The new height of the window
*/
/******************************************************************************/
void Renderer::Resize(GLFWwindow* window, int w, int h)
{
    mainCam.width = w;
    mainCam.height = h;

    // Update the viewport and any relevant projection matrices
    glViewport(0, 0, w, h);

    // TODO:: any additional resizing code for the camera ...

    if (window) {
        glfwSetWindowSize(window, w, h);
    }
}

void Rendering::Renderer::SetUpDeferredGeomUniformLocations()
{
    GLuint prog = m_shaders[TO_INT(ProgType::DEFERRED_GEOMPASS)].GetProgramID();
    m_gForwardRenderOnLoc = glGetUniformLocation(prog,"forwardRenderOn");
    m_gMVMatLoc = glGetUniformLocation(prog, "mvMat");
    m_gNMVMatLoc = glGetUniformLocation(prog, "nmvMat");
    m_gProjMatLoc = glGetUniformLocation(prog, "projMat");
    m_gNumLightsLoc = glGetUniformLocation(prog, "numLights");
    m_gObjectTypeLoc = glGetUniformLocation(prog, "objType");
    m_gNormalMappingOnLoc = glGetUniformLocation(prog, "normalMappingOn");
    m_gParallaxMappingOnLoc = glGetUniformLocation(prog, "parallaxMappingOn");
    m_gColorTexLoc = glGetUniformLocation(prog, "colorTex");
    m_gNormalTexLoc = glGetUniformLocation(prog, "normalTex");
    m_gBumpTexLoc = glGetUniformLocation(prog, "bumpTex");
    m_gAmbientLoc = glGetUniformLocation(prog, "ambient");
    m_gSpecularPowerLoc = glGetUniformLocation(prog, "specularPower");

    for (int i = 0; i < NUM_MAX_LIGHTS; ++i) {
        std::string index = std::to_string(i);
        m_gLightPosVFLoc[i]= glGetUniformLocation(prog, ("lightPosVF[" + index + "]").c_str());
        m_gDiffuseLoc[i] = glGetUniformLocation(prog, ("diffuse[" + index + "]").c_str());
        m_gSpecularLoc[i] = glGetUniformLocation(prog, ("specular[" + index + "]").c_str());
    }
}

void Rendering::Renderer::SetUpDeferredLightUniformLocations() {
	GLuint prog = m_shaders[TO_INT(ProgType::DEFERRED_LIGHTPASS)].GetProgramID();
    //m_lLightPassQuadLoc = glGetUniformLocation(prog, "");
    //TODO::delete tex locs
	m_lLightPassDebugLoc = glGetUniformLocation(prog, "lightPassDebug");
	m_lColorTexLoc = glGetUniformLocation(prog, "colorTex");
    m_lNumLightsLoc = glGetUniformLocation(prog, "numLights");
	m_lPosTexLoc = glGetUniformLocation(prog, "posTex");
	m_lNrmTexLoc = glGetUniformLocation(prog, "nrmTex");
    m_lTanTexLoc = glGetUniformLocation(prog, "tanTex");
    m_lDepthTexLoc = glGetUniformLocation(prog, "depthTex");
    m_lAmbientLoc = glGetUniformLocation(prog, "ambient");
    m_lSpecularPowerLoc= glGetUniformLocation(prog, "specularPower");
    m_lBlinnPhongLightingLoc = glGetUniformLocation(prog, "blinnPhongLighting");
    m_lParallaxMappingOnLoc = glGetUniformLocation(prog, "parallaxMappingOn");
    m_lNormalMappingObjTypeLoc = glGetUniformLocation(prog, "normalMappingObjType");
    for (int i = 0; i < NUM_MAX_LIGHTS; ++i) {
        std::string index = std::to_string(i);
        m_lDiffuseLoc[i] = glGetUniformLocation(prog, ("diffuse[" + index + "]").c_str());
        m_lSpecularLoc[i] = glGetUniformLocation(prog, ("specular[" + index + "]").c_str());
        m_lLightPosVFLoc[i] = glGetUniformLocation(prog, ("lightPosVF[" + index + "]").c_str());
    }
}


void Renderer::InitImGui() {
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark(); // or ImGui::StyleColorsClassic();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(m_window.get(), true);
    ImGui_ImplOpenGL3_Init("#version 330");
}

void Renderer::InitRendering() {
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD\n";
        exit(EXIT_FAILURE);
    }

    // Set up viewport
    int width, height;
    glfwGetFramebufferSize(m_window.get(), &width, &height);
    glViewport(0, 0, width, height);
}

void Rendering::Renderer::SendLightProperties(Core::Scene& scene, int lightIdx)
{
    if (lightIdx >= scene.GetNumLights()) {
        throw std::runtime_error("SendLightProperties::light index out of range");
    }

	const int NumLights = scene.GetNumLights();
    for (int i = 0; i < NumLights; ++i) {
        scene.m_lightPosVF[i] = Vec3(m_mainCamViewMat * Vec4(scene.m_lightPosWF[i], 1.0f));
        glUniform3fv(m_lLightPosVFLoc[i], 1, ValuePtr(scene.m_lightPosVF[i]));
    }

    Vec4 diffuse = scene.m_I[lightIdx] * scene.m_diffuseAlbedo;
    Vec4 specular = scene.m_I[lightIdx] * scene.m_specularAlbedo;

    glUniform4fv(m_lDiffuseLoc[lightIdx], 1, ValuePtr(diffuse));
    glUniform4fv(m_lSpecularLoc[lightIdx], 1, ValuePtr(specular));
    
    //-----------------------------------------------------------
    m_shaders[TO_INT(ProgType::DEFERRED_GEOMPASS)].Use();
	for (int i = 0; i < NumLights; ++i) {
		glUniform3fv(m_gLightPosVFLoc[i], 1, ValuePtr(scene.m_lightPosVF[i]));
	}

    glUniform4fv(m_gDiffuseLoc[lightIdx], 1, ValuePtr(diffuse));
    glUniform4fv(m_gSpecularLoc[lightIdx], 1, ValuePtr(specular));

    m_shouldUpdateCubeMapForSphere = true;
}

void Rendering::Renderer::UpdateNumLights(int numLights){
    m_shaders[TO_INT(ProgType::DEFERRED_LIGHTPASS)].Use();
    glUniform1i(m_lNumLightsLoc, numLights);

    m_shaders[TO_INT(ProgType::DEFERRED_GEOMPASS)].Use();
    glUniform1i(m_gNumLightsLoc, numLights);
}

// Function to update the mapping when objects are added/removed
void Rendering::Renderer::UpdateGuiToObjectIndexMap(const Core::Scene& scene) {
    m_guiToObjectIndexMap.clear();
    for (size_t i = 0; i < scene.m_objects.size(); ++i) {
        if (scene.m_objects[i]->GetCollider()->GetCollisionEnabled() == true) {
            m_guiToObjectIndexMap.push_back(i);
        }
    }
}

// GLFW's window handling doesn't directly support smart pointers since the GLFW API is a C API that expects raw pointers. 
// therefore, provided a custom deleter for the std::unique_ptr to properly handle GLFW window destruction.
void Renderer::WindowDeleter(GLFWwindow* window) {
    glfwDestroyWindow(window);
    glfwTerminate();// terminate GLFW after all resources are released
}

/******************************************************************************/
/*!
\fn     void RenderSkybox(const Mat4 &viewMat)
\brief
        Render the background using skybox cubemap.
\param  viewMat
        The view transform matrix to define the orientation of our camera.
*/
/******************************************************************************/
void Renderer::RenderSkybox(const Mat4& viewMat)
{
    glClearBufferfv(GL_DEPTH, 0, &one);

    m_shaders[TO_INT(ProgType::SKYBOX_PROG)].Use();

    SendCubeTexID(ResourceManager::GetInstance().m_skyboxTexID, m_skyboxTexCubeLoc);
    SendViewMat(viewMat, m_skyboxViewMatLoc);

    /*  Just trigger the skybox shaders, which hard-code the full-screen quad drawing */
    /*  No vertices are actually sent */
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}


/******************************************************************************/
/*!
\fn     void RenderObj(const Object &obj)
\brief
        Render an object.
\param  obj
        The object that we want to render.
*/
/******************************************************************************/
void Renderer::RenderObj(const Core::Object& obj)
{
    /*  Tell shader to use obj's VAO for rendering */
    const Mesh& mesh = *obj.GetMesh();
    glBindVertexArray(mesh.VAO);
    glDrawElements(GL_TRIANGLES, mesh.numIndices, GL_UNSIGNED_INT, nullptr);
}


/******************************************************************************/
/*!
\fn     void RenderSphere()
\brief
        Render the sphere using its own shader program.
*/
/******************************************************************************/
void Renderer::RenderSphere(const Core::Scene& scene)
{
    if (scene.m_sphere == nullptr) {
        return; // no sphere to render
    }

    m_shaders[TO_INT(ProgType::SPHERE_PROG)].Use();

    SendCubeTexID(ResourceManager::GetInstance().m_sphereTexID, m_sphereTexCubeLoc);

    /*  Indicate whether we want reflection/refraction or both */
    glUniform1i(m_sphereRefLoc, TO_INT(m_sphereRef));

    /*  Set refractive index of the sphere */
    glUniform1f(m_sphereRefIndexLoc, m_sphereRefIndex);

    /*  We need view mat to know our camera orientation */
    SendViewMat(m_mainCamViewMat, m_sphereViewMatLoc);

    // compute and send the model-view matrix for the sphere
    Mat4 sphereMV = m_mainCamViewMat * scene.m_sphere->GetModelMatrix();
    Mat4 sphereNMV = Transpose(Inverse(sphereMV));
    SendMVMat(sphereMV, sphereNMV, m_sphereMVMatLoc, m_sphereNMVMatLoc);

    // send the projection matrix
    SendProjMat(m_mainCamProjMat, m_sphereProjMatLoc);

    // render the sphere
    RenderObj(*scene.m_sphere);
}


/******************************************************************************/
/*!
\fn     void RenderObjsBg(  Mat4 MVMat[], Mat4 normalMVMat[],
                            Mat4 viewMat, Mat4 projMat,
                            int viewportWidth, int viewportHeight,
                            int renderPass)
\brief
        Render background and objects in the scene, except the sphere.
\param  MVMat
        The mode-view matrices of the objects.
\param  normalMVMat
        The normal mode-view matrices of the normals on the objects.
\param  viewMat
        The view transform matrix of the camera to render the skybox background.
\param  projMat
        The projection matrix of the camera.
\param  viewportWidth
        The width of the viewport to render to.
\param  viewportHeight
        The height of the viewport to render to.
\param  renderPass
        Whether we are rendering textures for sphere reflection/refraction, or
        mirror reflection, or rendering objects onto the screen.
        We need this flag because each pass only render certain objects.
*/
/******************************************************************************/
void Renderer::RenderObjects(RenderPass renderPass, Core::Scene& scene, int faceIdx)
{
    /*  We need to set this here because the onscreen rendering will use a bigger viewport than
        the rendering of sphere/mirror reflection/refraction texture
    */
    ResourceManager& resourceManager = ResourceManager::GetInstance();
    if (renderPass == RenderPass::NORMAL) {
        glViewport(0, 0, mainCam.width, mainCam.height);
        RenderSkybox(m_mainCamViewMat);
        m_shaders[TO_INT(ProgType::DEFERRED_GEOMPASS)].Use();
        SendProjMat(m_mainCamProjMat, m_gProjMatLoc);
    }
    else if (renderPass == RenderPass::MIRRORTEX_GENERATION) {
        glViewport(0, 0, mirrorCam.width, mirrorCam.height);
        RenderSkybox(m_mirrorCamViewMat);
        m_shaders[TO_INT(ProgType::DEFERRED_GEOMPASS)].Use();
        SendProjMat(m_mirrorCamProjMat, m_gProjMatLoc);
    }
    else if (renderPass == RenderPass::SPHERETEX_GENERATION) {
        glViewport(0, 0, resourceManager.m_skyboxFaceSize, resourceManager.m_skyboxFaceSize);
        RenderSkybox(m_sphereCamViewMat[faceIdx]);
        m_shaders[TO_INT(ProgType::DEFERRED_GEOMPASS)].Use();
        SendProjMat(m_sphereCamProjMat, m_gProjMatLoc);
    }


    /*  Send object texture and render them */
    const size_t numObjs = scene.m_objects.size();
    for (int i{}; i < numObjs; ++i) {
        const auto& obj = *scene.m_objects[i];
        if (obj.IsVisible() == false) {
            continue;
        }        
        glUniform1i(m_gObjectTypeLoc, TO_INT(obj.GetObjType()));
        glUniform1i(m_gForwardRenderOnLoc, obj.GetObjType()!=Core::ObjectType::DEFERRED_REGULAR);
        if (obj.GetObjType() == Core::ObjectType::REFLECTIVE_CURVED && renderPass == RenderPass::MIRRORTEX_GENERATION) {//spherical mirror
            continue;           /*  Will use sphere rendering program to apply reflection & refraction textures on sphere */
        }
        else
        {
            //if (renderPass == RenderPass::MIRRORTEX_GENERATION && (obj.GetObjType() == Core::ObjectType::REFLECTIVE_FLAT))
            //{
            //    continue;           /*  Not drawing objects behind mirror & mirror itself */
            //}
            //else
            {
                //if (renderPass == RenderPass::SPHERETEX_GENERATION && (obj.GetObjType() == Core::ObjectType::REFLECTIVE_FLAT)) {
                //    continue;           /*  Not drawing mirror when generating reflection/refraction texture for sphere to avoid inter-reflection */
                //}
                //else
                {
                    if (obj.GetObjType() == Core::ObjectType::REFLECTIVE_FLAT)
                    {
                        SendMirrorTexID();
                    }
                    else
                    {
                        SendObjTexID(resourceManager.GetTexture(obj.GetImageID()), TO_INT(ActiveTexID::COLOR), m_gColorTexLoc);
                    }

                    if (renderPass == RenderPass::NORMAL) {
						SendMVMat(m_mainCamMVMat[i], m_mainCamNormalMVMat[i], m_gMVMatLoc, m_gNMVMatLoc);
                    }
                    else if (renderPass == RenderPass::MIRRORTEX_GENERATION) {
                        SendMVMat(m_mirrorCamMVMat[i], m_mirrorCamNormalMVMat[i], m_gMVMatLoc, m_gNMVMatLoc);
                    }
                    else if (renderPass == RenderPass::SPHERETEX_GENERATION) {
                        SendMVMat(m_sphereCamMVMat[i][faceIdx], m_sphereCamNormalMVMat[i][faceIdx], m_gMVMatLoc, m_gNMVMatLoc);
                    }

                    if (obj.GetObjType() == Core::ObjectType::NORMAL_MAPPED_PLANE)   /*  apply normal mapping / parallax mapping for the base */
                    {
                        SendObjTexID(resourceManager.m_normalTexID, TO_INT(ActiveTexID::NORMAL), m_gNormalTexLoc);
                        glUniform1i(m_gNormalMappingOnLoc, true);
                        glUniform1i(m_gParallaxMappingOnLoc, m_parallaxMappingOn);

                        SendObjTexID(resourceManager.m_bumpTexID, TO_INT(ActiveTexID::BUMP), m_gBumpTexLoc);
                        //if (m_parallaxMappingOn) {
                        //}
                    }
                    else                       /*  not apply normal mapping / parallax mapping for other objects */
                    {
                        glUniform1i(m_gNormalMappingOnLoc, false);
                        glUniform1i(m_gParallaxMappingOnLoc, false);
                    }

                    /*  The mirror surface is rendered to face away to simulate the flipped effect.
                        Hence we need to perform front-face culling for it.
                        Other objects use back-face culling as usual.
                    */
                    if (obj.GetObjType() == Core::ObjectType::REFLECTIVE_FLAT) {
                        glCullFace(GL_FRONT);
                    }

                    RenderObj(obj);

                    /*  Trigger back-face culling again */
                    if (obj.GetObjType() == Core::ObjectType::REFLECTIVE_FLAT) {
                        glCullFace(GL_BACK);
                    }
                }
            }
        }
    }
}

/******************************************************************************/
/*!
\fn     void RenderToSphereCubeMapTexture(unsigned char *sphereCubeMapTexture[])
\brief
        Render the scene to 6 faces of the cubemap for sphere
        reflection/refraction later.
\param  sphereCubeMapTexture
        Buffers to store the 6 faces of the cubemap texture.
*/
/******************************************************************************/
void Renderer::RenderToSphereCubeMapTexture(Core::Scene& scene)
{
    /*  Theoretically the rendering to cubemap texture can be done in the same way as 2D texture:
        rendering straight to the GPU cubemap texture object, similar to what we do for the
        2D mirror texture in RenderToMirrorTexture.
        However, some graphics drivers don't seem to implement the framebuffer cubemap texture properly.
        So we do the cubemap texture writing manually here: copy the framebuffer to CPU texture data,
        then copy that data to GPU texture object later (in SetUpSphereTexture function).
    */
    ResourceManager& resourceManager = ResourceManager::GetInstance();

    GLuint sphereFrameBufferID;
    glGenFramebuffers(1, &sphereFrameBufferID);
    glBindFramebuffer(GL_FRAMEBUFFER, sphereFrameBufferID);

    GLuint sphereFrameBufferTexID;
    glGenTextures(1, &sphereFrameBufferTexID);
    glBindTexture(GL_TEXTURE_2D, sphereFrameBufferTexID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, resourceManager.m_skyboxFaceSize, resourceManager.m_skyboxFaceSize,0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

    for (int i = 0; i < TO_INT(CubeFaceID::NUM_FACES); ++i)
    {
        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, sphereFrameBufferTexID, 0);

        RenderObjects(RenderPass::SPHERETEX_GENERATION, scene,i);

        glReadBuffer(GL_COLOR_ATTACHMENT0);
        glReadPixels(0, 0, resourceManager.m_skyboxFaceSize, resourceManager.m_skyboxFaceSize, GL_RGBA, GL_UNSIGNED_BYTE, resourceManager.m_sphereCubeMapData[i].get());
    }

    glDeleteTextures(1, &sphereFrameBufferTexID);
    glDeleteFramebuffers(1, &sphereFrameBufferID);

    resourceManager.SetUpSphereCubeMapTexture();
}


/******************************************************************************/
/*!
\fn     void RenderToMirrorTexture(Core::Scene& scene)
\brief
        Render the scene to the texture for mirror reflection. This texture was
        already bound to mirrorFrameBufferID in SetUpMirrorTexture function.
*/
/******************************************************************************/
void Renderer::RenderToMirrorTexture(Core::Scene& scene)
{
    glBindFramebuffer(GL_FRAMEBUFFER, ResourceManager::GetInstance().m_mirrorFrameBufferID);
    RenderObjects(RenderPass::MIRRORTEX_GENERATION,scene);
}


/******************************************************************************/
/*!
\fn     void RenderToScreen(Core::Scene& scene)
\brief
        Render the scene to the default framebuffer.
*/
/******************************************************************************/
void Renderer::RenderToScreen(Core::Scene& scene)
{
    glBindFramebuffer(GL_FRAMEBUFFER, m_gFrameBufferID);
    RenderObjects(RenderPass::NORMAL,scene);
}

/******************************************************************************/
/*!
\fn     void Render()
\brief
        Render function for update & drawing.
        This function will perform forward shading if deferredShading == false.
        Otherwise it will perform geometry pass followed by light pass of
        deferred shading.
*/
/******************************************************************************/
void Renderer::Render(Core::Scene& scene, float fps)
{
    ComputeMainCamMats(scene);
    ComputeMirrorCamMats(scene);

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    //(1) Geometry Pass
    {

        m_shaders[TO_INT(ProgType::DEFERRED_GEOMPASS)].Use();

        // Bind G-buffer framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, m_gFrameBufferID);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};
        glDrawBuffers(3, drawBuffers);

        GLfloat bgColor[4] = { 1.f, 1.2f, 1.f, 1.0f }; 
        glClearBufferfv(GL_COLOR, 0, bgColor);//color
        glClearBufferfv(GL_COLOR, 1, glm::value_ptr(glm::vec4(0.0f)));//pos
        glClearBufferfv(GL_COLOR, 2, glm::value_ptr(glm::vec4(0.0f)));//normal
        glClearBufferfv(GL_DEPTH, 0, &one);                           //depth
    }

	//-------------------
    //(2) rendering objects 
    if (scene.m_sphere &&
        (ShouldUpdateSphereCubemap(scene.m_sphere->GetRigidBody()->GetLinearVelocity().LengthSquared()) == true))
    {
        ComputeSphereCamMats(scene);

        /*  Theoretically the rendering to cubemap texture can be done in the same way as 2D texture:
            rendering straight to the GPU texture object, similar to what we do for the
            2D mirror texture below.
            However, some graphics drivers don't seem to implement the framebuffer cubemap texture properly.
            So we do the cubemap texture generation manually here: copy the framebuffer to CPU texture data,
            then copy that data to the GPU texture object.
        */
        RenderToSphereCubeMapTexture(scene);

        m_shouldUpdateCubeMapForSphere = false;
    }

    /*  The texture for planar reflection is view-dependent, so it needs to be rendered on the fly,
        whenever the mirror is visible and camera is moving
    */
    if (m_mirrorVisible && (mainCam.moved || mirrorCam.moved)) {
        RenderToMirrorTexture(scene);
    }

    /*  Render the scene, except the sphere to the screen */
    RenderToScreen(scene);
    /*  This is done separately, as it uses a different shader program for reflection/refraction */
    RenderSphere(scene);
    //----------------------
    // (3) light pass
    {
        m_shaders[TO_INT(ProgType::DEFERRED_LIGHTPASS)].Use();

        /*  Bind framebuffer to 0 to render to the screen */
        /*  Disable depth test since we only render flat textures */
        /*  Disable writing to depth buffer */
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        
        glClearColor(0.f, 0.f, 0.f, 1.f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glDisable(GL_DEPTH_TEST);
        glDepthMask(GL_FALSE);

        // Bind the color texture to texture unit 0
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_gColorTexID);        
        glUniform1i(m_lColorTexLoc, 0);

        // Bind the position texture to texture unit 1
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, m_gPosTexID);
        glUniform1i(m_lPosTexLoc,1);

        // Bind the normal texture to texture unit 2
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, m_gNrmTexID);
        glUniform1i(m_lNrmTexLoc, 2);
        
         //Bind the depth texture to texture unit 3
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, m_gDepthTexID);
        glUniform1i(m_lDepthTexLoc, 3);


        glBindVertexArray(quadVAO[TO_INT(DebugType::MAIN)]);
        glUniform1i(m_lLightPassDebugLoc, TO_INT(DebugType::MAIN));
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        if (m_buffersDisplay)
        {
            for (int i = TO_INT(DebugType::COLOR); i < TO_INT(DebugType::NUM_DEBUGTYPES); ++i)
            {
                glUniform1i(m_lLightPassDebugLoc, i);
                /*  Send corresponding quads to shader for rendering
                    debugging minimaps.
                    Appropriate flag (COLOR/POSITION/NORMAL/DEPTH) should
                    also be sent to shader to tell it which buffer to
                    display
                */
                switch (i)
                {
                case TO_INT(DebugType::COLOR):
                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D, m_gColorTexID);
                    break;
                case TO_INT(DebugType::POSITION):
                    glActiveTexture(GL_TEXTURE1);
                    glBindTexture(GL_TEXTURE_2D, m_gPosTexID);
                    break;
                case TO_INT(DebugType::NORMAL):
                    glActiveTexture(GL_TEXTURE2);
					glBindTexture(GL_TEXTURE_2D, m_gNrmTexID);
                    break;
                case TO_INT(DebugType::DEPTH):
                    glActiveTexture(GL_TEXTURE3);
                    glBindTexture(GL_TEXTURE_2D, m_gDepthTexID);
                    break;
                case TO_INT(DebugType::NORMAL_MAPPING_MASK):
                    glActiveTexture(GL_TEXTURE2);
                    glBindTexture(GL_TEXTURE_2D, m_gNrmTexID);
                    break;
                }
                glBindVertexArray(quadVAO[i]);
                glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
            }
        }

        /*  Enable depth test again for rendering objects in the next frame */
        glEnable(GL_DEPTH_TEST);

        /*  Enable writing to depth buffer */
        glDepthMask(GL_TRUE);
    }

    //------------------------------------------------------------
    RenderGui(scene, fps);

    // Rendering    
    ImGui::Render();
    int display_W, display_H;
    glfwGetFramebufferSize(m_window.get(), &display_W, &display_H);
    glViewport(0, 0, display_W, display_H);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    mainCam.moved = false;
    mainCam.resized = false;
    mirrorCam.moved = true;

    glfwSwapBuffers(m_window.get());
}

Rendering::Renderer::~Renderer() {
    CleanUp(); // free all resources
}