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
#include <rendering/Scene.h>
#include <rendering/ResourceManager.h>
#include <input/input.h>
#include <math/Math.h>
#include <utilities/ToUnderlyingEnum.h>
#include <image_io.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <ctime>

#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

using namespace Rendering;
using Core::Object;


/******************************************************************************/
/*!
\fn     void SetUpSkyBoxUniformLocations(GLuint prog)
\brief
        Look up the locations of uniform variables in the skybox shader program.
\param  prog
        The given skybox shader program ID.
*/
/******************************************************************************/
void Renderer::SetUpSkyBoxUniformLocations(GLuint prog)
{
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
void Renderer::SetUpMainUniformLocations(GLuint prog)
{
    m_mainMVMatLoc = glGetUniformLocation(prog, "mvMat");
    m_mainNMVMatLoc = glGetUniformLocation(prog, "nmvMat");
    m_mainProjMatLoc = glGetUniformLocation(prog, "projMat");

    m_textureLoc = glGetUniformLocation(prog, "colorTex");

    m_numLightsLoc = glGetUniformLocation(prog, "numLights");
    m_lightPosLoc = glGetUniformLocation(prog, "lightPosVF");
    m_lightOnLoc = glGetUniformLocation(prog, "lightOn");

    m_ambientLoc = glGetUniformLocation(prog, "ambient");
    m_diffuseLoc = glGetUniformLocation(prog, "diffuse");
    m_specularLoc = glGetUniformLocation(prog, "specular");
    m_specularPowerLoc = glGetUniformLocation(prog, "specularPower");

    m_bumpTexLoc = glGetUniformLocation(prog, "bumpTex");
    m_normalTexLoc = glGetUniformLocation(prog, "normalTex");
    m_normalMappingOnLoc = glGetUniformLocation(prog, "normalMappingOn");
    m_parallaxMappingOnLoc = glGetUniformLocation(prog, "parallaxMappingOn");
}


/******************************************************************************/
/*!
\fn     void SetUpSphereUniformLocations(GLuint prog)
\brief
        Look up the locations of uniform variables in the sphere shader program.
\param  prog
        The given sphere shader program ID.
*/
/******************************************************************************/
void Renderer::SetUpSphereUniformLocations(GLuint prog)
{
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

/******************************************************************************/
/*!
\fn     void SendLightProperties()
\brief
        Send numLights and intensities to the rendering program.
*/
/******************************************************************************/
void Renderer::SendLightProperties(const Scene& scene)
{
    glUniform1i(m_numLightsLoc, scene.NUM_LIGHTS);

    /*  ambient, diffuse, specular are now reflected components on the object
        surface and can be used directly as intensities in the lighting equation.
    */
    Vec4 ambient, diffuse, specular;
    ambient = scene.m_I * scene.m_ambientAlbedo;
    diffuse = scene.m_I * scene.m_diffuseAlbedo;
    specular = scene.m_I * scene.m_specularAlbedo;

    glUniform4fv(m_ambientLoc, 1, ValuePtr(ambient));
    glUniform4fv(m_diffuseLoc, 1, ValuePtr(diffuse));
    glUniform4fv(m_specularLoc, 1, ValuePtr(specular));

    glUniform1i(m_specularPowerLoc, scene.m_specularPower);
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
void Renderer::ComputeObjMVMats(Mat4* MVMat, Mat4* NMVMat,const Mat4& viewMat, const Scene& scene)
{
    const size_t OBJ_SIZE = TO_INT(ObjID::NUM_OBJS);
    for (int i = 0; i < OBJ_SIZE; ++i)
    {
        Mat4 objMat = scene.m_objects[i]->GetModelMatrixGLM();
        MVMat[i] = viewMat * objMat;
        NMVMat[i] = Transpose(Inverse(MVMat[i]));
    }
}


/******************************************************************************/
/*!
\fn     void ComputeMainCamMats()
\brief
        Compute the view/projection and other related matrices for user camera.
*/
/******************************************************************************/
void Renderer::ComputeMainCamMats(const Scene& scene)
{
    /*  Update projection matrix */
    if (mainCam.resized) {
        m_mainCamProjMat = mainCam.ProjMat();
    }

    /*  Update view transform matrix */
    //if (mainCam.moved){
        m_mainCamViewMat = mainCam.ViewMat();
        ComputeObjMVMats(m_mainCamMVMat, m_mainCamNormalMVMat, m_mainCamViewMat, scene);
    //}

}


/******************************************************************************/
/*!
\fn     void ComputeMirrorCamMats(const Scene& scene)
\brief
        Compute the view/projection and other related matrices for mirror camera.
*/
/******************************************************************************/
void Renderer::ComputeMirrorCamMats(const Scene& scene)
{
    if (mainCam.moved)
    {
        /*  Computing position of user camera in mirror frame */
        Vec3 mainCamMirrorFrame = Vec3(Rotate(-scene.m_mirrorRotationAngle, scene.m_mirrorRotationAxis) 
                                        *Translate(-scene.m_mirrorTranslate) * Vec4(mainCam.pos, 1.0));

        /*  If user camera is behind mirror, then mirror is not visible and no need to compute anything */
        if (mainCamMirrorFrame.z <= 0)
        {
            m_mirrorVisible = false;
            return;
        }
        else
            m_mirrorVisible = true;

        /*  In mirror frame, mirror camera position is defined as (x, y, -z) in which (x, y, z) is the
            user camera position in mirror frame.
            We also need to compute mirrorCam.pos, mirrorCam.upVec, mirrorCam.lookAt are defined in world
            frame to compute mirror cam's view matrix.
            function to compute m_mirrorCamViewMat
        */

        Vec3 mirrorCamMirrorFrame = Vec3(mainCamMirrorFrame.x, mainCamMirrorFrame.y, -mainCamMirrorFrame.z);

        mirrorCam.pos = Vec3(Translate(scene.m_mirrorTranslate) * Rotate(scene.m_mirrorRotationAngle, scene.m_mirrorRotationAxis) * Vec4(mirrorCamMirrorFrame, 1.0));
        mirrorCam.upVec = BASIS[Y];
        mirrorCam.lookAt = Vec3(Translate(scene.m_mirrorTranslate) * Rotate(scene.m_mirrorRotationAngle, scene.m_mirrorRotationAxis) * Vec4(0, 0, 0, 1));

        m_mirrorCamViewMat = LookAt(mirrorCam.pos, mirrorCam.lookAt, mirrorCam.upVec);

        ComputeObjMVMats(m_mirrorCamMVMat, m_mirrorCamNormalMVMat, m_mirrorCamViewMat,scene);
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
        std::vector<Vec3> midsPoint = {
            Vec3(0.5, 0, 0), // Left
            Vec3(-0.5, 0, 0), // Right
            Vec3(0, -0.5, 0), // Bottom
            Vec3(0, 0.5, 0) // Top
        };


        float nearDist = INFINITY;
        for (Vec3& midPoint : midsPoint) {
            Vec3 midCamFrame = Vec3(Translate(scene.m_mirrorTranslate) * Rotate(scene.m_mirrorRotationAngle, scene.m_mirrorRotationAxis) * Vec4(midPoint, 1.0));//mid : mirrorFrame -> cameraFrame
            midPoint = midCamFrame - mirrorCam.pos;//mid : cameraFrame -> mirroredCameraFrame
            float projectionLength = Dot(midPoint, mirrorCamViewMirrorFrame);
            nearDist = std::min(nearDist, projectionLength);
        }
        // Note that midsPoint are now in mirrorCameraFrame

        //(minor)
        // Setting the far plane to infinity can lead to depth precision issues, causing distant objects to dominate in reflections. 
        // A minimum near plane distance is set to mitigate this, particularly suitable for this static scene scenarios.
        //mirrorCam.nearPlane = std::max(nearDist, 2.5f);
        constexpr float MAX_FAR_PLANE = 100.f;
        constexpr float MIN_NEAR_PLANE = 0.1f;

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

        Vec3 left = ComputeIntersectionOnNearPlane(midsPoint[0]);
        Vec3 right = ComputeIntersectionOnNearPlane(midsPoint[1]);
        Vec3 bottom = ComputeIntersectionOnNearPlane(midsPoint[2]);
        Vec3 top = ComputeIntersectionOnNearPlane(midsPoint[3]);

        mirrorCam.leftPlane = -(toNearMirrorCamFrame - left).length();
        mirrorCam.rightPlane = (toNearMirrorCamFrame - right).length();
        mirrorCam.bottomPlane = -(toNearMirrorCamFrame - bottom).length();
        mirrorCam.topPlane = (toNearMirrorCamFrame - top).length();

        float viewAngleAdjustFactor = 0.8f; //scales down the frustum planes
        mirrorCam.leftPlane *= viewAngleAdjustFactor;
        mirrorCam.rightPlane *= viewAngleAdjustFactor;
        mirrorCam.bottomPlane *= viewAngleAdjustFactor;
        mirrorCam.topPlane *= viewAngleAdjustFactor;
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
void Renderer::ComputeSphereCamMats(const Scene& scene)
{
    /*  Compute the lookAt positions for the 6 faces of the sphere cubemap.
        The sphere camera is at spherePos.
        The front and back faces are -z and +z.
        The left and right faces are -x and +x.
        The bottom and top faces are -y and +y.
    */
    // Directions for the cubemap faces
    Vec3 lookAt[TO_INT(CubeFaceID::NUM_FACES)] = {
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
    Vec3 upVec[TO_INT(CubeFaceID::NUM_FACES)] = {
        -BASIS[1], // RIGHT flipped
        -BASIS[1], // LEFT flipped
        BASIS[2],  // TOP flipped
        -BASIS[2], // BOTTOM flipped
        -BASIS[1], // BACK flipped
        -BASIS[1]  // FRONT flipped
    };


    for (int f = 0; f < TO_INT(CubeFaceID::NUM_FACES); ++f)
    {
        m_sphereCamViewMat[f] = LookAt(scene.m_spherePos, scene.m_spherePos + lookAt[f], upVec[f]);
        ComputeObjMVMats(m_sphereCamMVMat[f], m_sphereCamNormalMVMat[f], m_sphereCamViewMat[f],scene);
    }


    /*  Use Perspective function to ompute the projection matrix m_sphereCamProjMat so that
        from the camera position at the cube center, we see a complete face of the cube.
        The near plane distance is 0.01f. The far plane distance is equal to mainCam's farPlane.
    */
    // Compute the projection matrix for the sphere camera
    float fov = PI / 2.f; // 90 degrees in radians
    float aspectRatio = 1.0f;
    float nearPlane = 0.01f; // near plane is 0.01, and far plane is the same as the main camera's far plane
    m_sphereCamProjMat = Perspective(fov, aspectRatio, nearPlane, mainCam.farPlane);
}

inline void Rendering::Renderer::RenderGui(float fps) {
    ImGui::SetNextWindowPos(ImVec2(Camera::DISPLAY_SIZE - Camera::GUI_WIDTH, 0));
    ImGui::SetNextWindowSize(ImVec2(Camera::GUI_WIDTH, Camera::GUI_WIDTH * 0.3));
    // Displaying FPS
    ImGui::Text("Frame Rate: %.1f", fps); // Assuming fps is a float variable

    int refTypeInt = static_cast<int>(m_sphereRef);
    const char* refTypes[] = { "Reflection Only", "Refraction Only", "Reflection & Refraction" };
    if (ImGui::Combo("Sphere", &refTypeInt, refTypes, IM_ARRAYSIZE(refTypes))) {
        m_sphereRef = static_cast<RefType>(refTypeInt); // Cast back to enum class after ImGui interaction
    }

    ImGui::SliderFloat("Sphere Refractive Index", &m_sphereRefIndex, 1.0f, 2.5f); // Adjust the range as needed
    // Parallax mapping toggling
    ImGui::Checkbox("Parallax Mapping", &Renderer::GetInstance().GetParallaxMapping());

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
void SendProjMat(Mat4 projMat, GLint projMatLoc)
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
    glBindTexture(GL_TEXTURE_2D, ResourceManager::GetInstance().mirrorTexID);
    glUniform1i(m_textureLoc, 0);
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
void Renderer::AttachScene(const Scene& scene)
{
    for (const auto& pair : m_shaderFileMap) {
        m_shaders[TO_INT(pair.first)].LoadShader(pair.second.vertexShaderPath, pair.second.fragmentShaderPath);
    }

    /*  Send mesh data only */
    ResourceManager& resourceManager = ResourceManager::GetInstance();
    size_t NUM_MESHES = TO_INT(MeshID::NUM_MESHES);
    for (int i = 0; i < NUM_MESHES; ++i) {
        Mesh& mesh = resourceManager.GetMesh(static_cast<MeshID>(i));
        SetUpVertexData(mesh);
    }

    resourceManager.SetUpTextures();

    /*  Look up for the locations of the uniform variables in the shader programs */
    // For SKYBOX_PROG
    m_shaders[TO_INT(ProgType::SKYBOX_PROG)].Use();
    SetUpSkyBoxUniformLocations(m_shaders[TO_INT(ProgType::SKYBOX_PROG)].GetProgramID());

    // For SPHERE_PROG
    m_shaders[static_cast<int>(ProgType::SPHERE_PROG)].Use();
    SetUpSphereUniformLocations(m_shaders[TO_INT(ProgType::SPHERE_PROG)].GetProgramID());

    // For MAIN_PROG
    m_shaders[static_cast<int>(ProgType::MAIN_PROG)].Use();
    SetUpMainUniformLocations(m_shaders[TO_INT(ProgType::MAIN_PROG)].GetProgramID());
    SendLightProperties(scene);

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
        Mesh& mesh = resourceManager.GetMesh(static_cast<MeshID>(i));
        glDeleteVertexArrays(1, &mesh.VAO);
        glDeleteBuffers(1, &mesh.VBO);
        glDeleteBuffers(1, &mesh.IBO);
    }

    glDeleteTextures(TO_INT(ImageID::NUM_IMAGES), resourceManager.textureIDs.data());
    glDeleteTextures(1, &resourceManager.bumpTexID);
    glDeleteTextures(1, &resourceManager.normalTexID);
    glDeleteTextures(1, &resourceManager.skyboxTexID);
    glDeleteTextures(1, &resourceManager.mirrorTexID);
    glDeleteTextures(1, &resourceManager.sphereTexID);

    glDeleteFramebuffers(1, &resourceManager.mirrorFrameBufferID);

}

Rendering::Renderer::Renderer()
    : m_window{ nullptr,WindowDeleter }, m_fps(0)
    , m_sphereRef(RefType::REFLECTION_ONLY)
    , m_parallaxMappingOn(true), m_sphereRefIndex{1.33f}//water by default
{
	// Initialize GLFW
	if (!glfwInit()) {
		std::cerr << "Failed to initialize GLFW\n";
		exit(EXIT_FAILURE);
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // Specify the GLFW version
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // Required for MacOS
#endif

	// Create a windowed mode window and its OpenGL context
	GLFWwindow* rawWindow = glfwCreateWindow(Camera::DISPLAY_SIZE, Camera::DISPLAY_SIZE, "RigidBodyLab", nullptr, nullptr);
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

	//need to flip cuz 'stb_image' assumes the image's origin is at the bottom-left corner, while many image formats store the origin at the top-left corner.

    m_shaderFileMap[ProgType::MAIN_PROG] = { "../RigidBodyLab/shaders/main.vs",  "../RigidBodyLab/shaders/main.fs" };
    m_shaderFileMap[ProgType::SKYBOX_PROG] = { "../RigidBodyLab/shaders/skybox.vs", "../RigidBodyLab/shaders/skybox.fs" };
    m_shaderFileMap[ProgType::SPHERE_PROG] = { "../RigidBodyLab/shaders/sphere.vs", "../RigidBodyLab/shaders/sphere.fs" };
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
        exit(-1);
    }

    // Set up viewport
    int width, height;
    glfwGetFramebufferSize(m_window.get(), &width, &height);
    glViewport(0, 0, width, height);

    // Enable OpenGL features
    glEnable(GL_DEPTH_TEST); // Enable depth testing for 3D
    glEnable(GL_CULL_FACE); // Enable face culling
    glCullFace(GL_BACK); // Cull back faces
    glFrontFace(GL_CCW); // Front faces are counter-clockwise
    glEnable(GL_BLEND); // Enable blending (for transparency)
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void Rendering::Renderer::UpdateLightPosViewFrame(Scene& scene)
{
    if (mainCam.moved)
    {
        for (int i = 0; i < scene.NUM_LIGHTS; ++i) {
            scene.m_lightPosVF[i] = Vec3(m_mainCamViewMat * Vec4(scene.m_lightPosWF[i], 1.0f));
        }

        glUniform3fv(m_lightPosLoc, scene.NUM_LIGHTS, ValuePtr(scene.m_lightPosVF[0]));
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

    SendCubeTexID(ResourceManager::GetInstance().skyboxTexID, m_skyboxTexCubeLoc);
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
    const Mesh& mesh = obj.GetMesh();
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
//void Renderer::RenderSphere(const Scene& scene)
//{
//    m_shaders[TO_INT(ProgType::SPHERE_PROG)].Use();
//
//    SendCubeTexID(ResourceManager::GetInstance().sphereTexID, m_sphereTexCubeLoc);
//
//    /*  Indicate whether we want reflection/refraction or both */
//    glUniform1i(m_sphereRefLoc, TO_INT(m_sphereRef));
//
//    /*  Set refractive index of the sphere */
//    glUniform1f(m_sphereRefIndexLoc, m_sphereRefIndex);
//
//    /*  We need view mat to know our camera orientation */
//    SendViewMat(m_mainCamViewMat, m_sphereViewMatLoc);
//
//    /*  These are for transforming vertices on the sphere */
//    SendMVMat(m_mainCamMVMat[TO_INT(ObjID::SPHERE)], m_mainCamNormalMVMat[TO_INT(ObjID::SPHERE)], m_sphereMVMatLoc, m_sphereNMVMatLoc);
//    SendProjMat(m_mainCamProjMat, m_sphereProjMatLoc);
//
//    RenderObj(scene.GetObject(TO_INT(ObjID::SPHERE)));
//}


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
void Renderer::RenderObjsBg(const Mat4 * MVMat, const Mat4 *normalMVMat, const Mat4& viewMat, const Mat4& projMat,
    int viewportWidth, int viewportHeight,
    RenderPass renderPass, Scene& scene)
{
    /*  We need to set this here because the onscreen rendering will use a bigger viewport than
        the rendering of sphere/mirror reflection/refraction texture
    */
    glViewport(0, 0, viewportWidth, viewportHeight);

    RenderSkybox(viewMat);

    m_shaders[TO_INT(ProgType::MAIN_PROG)].Use();

    UpdateLightPosViewFrame(scene);
    SendProjMat(projMat, m_mainProjMatLoc);

    ResourceManager& resourceManager = ResourceManager::GetInstance();

    //plane only for now
    //SendObjTexID(resourceManager.normalTexID, TO_INT(ActiveTexID::NORMAL), m_normalTexLoc);
    //glUniform1i(m_normalMappingOnLoc, true);
    //glUniform1i(m_parallaxMappingOnLoc, Renderer::GetInstance().IsParallaxMappingOn());

    //if (Renderer::GetInstance().IsParallaxMappingOn()) {
    //    SendObjTexID(resourceManager.bumpTexID, TO_INT(ActiveTexID::BUMP), m_bumpTexLoc);
    //}
    //SendObjTexID(resourceManager.GetTexture(scene.GetObject(TO_INT(ObjID::BASE)).GetImageID()), TO_INT(ActiveTexID::COLOR), m_textureLoc);
    //glUniform1i(m_lightOnLoc, 1);     /*  enable lighting for other objects */

    //SendMVMat(MVMat[TO_INT(ObjID::BASE)], normalMVMat[TO_INT(ObjID::BASE)], m_mainMVMatLoc, m_mainNMVMatLoc);

    //RenderObj(scene.GetObject(TO_INT(ObjID::BASE)));

    /*  Send object texture and render them */
    size_t NUM_OBJS = TO_INT(ObjID::NUM_OBJS);
    for (int i = 0; i < NUM_OBJS; ++i)
        //if (i == TO_INT(ObjID::SPHERE)) {
        //    continue;           /*  Will use sphere rendering program to apply reflection & refraction textures on sphere */
        //}
        //else
        {
            //if (renderPass == RenderPass::MIRRORTEX_GENERATION
            //    && (i == TO_INT(ObjID::MIRROR))
            //        )
            //        //|| i == TO_INT(ObjID::MIRRORBASE1)
            //        //|| i == TO_INT(ObjID::MIRRORBASE2) || i == TO_INT(ObjID::MIRRORBASE3))) 
            //{
            //    continue;           /*  Not drawing objects behind mirror & mirror itself */
            //}
            //else 
            {
                //if (renderPass == RenderPass::SPHERETEX_GENERATION && (i == TO_INT(ObjID::MIRROR))) {
                //    continue;           /*  Not drawing mirror when generating reflection/refraction texture for sphere to avoid inter-reflection */
                //}
                //else
                {
                    //if (i == TO_INT(ObjID::MIRROR))
                    //{
                    //    SendMirrorTexID();
                    //    glUniform1i(m_lightOnLoc, 0);     /*  disable lighting on mirror surface */
                    //}
                    //else
                    {
                        SendObjTexID(resourceManager.GetTexture(scene.GetObject(i).GetImageID()), TO_INT(ActiveTexID::COLOR), m_textureLoc);
                        glUniform1i(m_lightOnLoc, 1);     /*  enable lighting for other objects */
                    }

                    SendMVMat(MVMat[i], normalMVMat[i], m_mainMVMatLoc, m_mainNMVMatLoc);

                    if (i == TO_INT(ObjID::BASE))   /*  apply normal mapping / parallax mapping for the base */
                    {
                        SendObjTexID(resourceManager.normalTexID, TO_INT(ActiveTexID::NORMAL), m_normalTexLoc);
                        glUniform1i(m_normalMappingOnLoc, true);
                        glUniform1i(m_parallaxMappingOnLoc, Renderer::GetInstance().IsParallaxMappingOn());

                        if (Renderer::GetInstance().IsParallaxMappingOn()) {
                            SendObjTexID(resourceManager.bumpTexID, TO_INT(ActiveTexID::BUMP), m_bumpTexLoc);
                        }
                    }
                    else                       /*  not apply normal mapping / parallax mapping for other objects */
                    {
                        glUniform1i(m_normalMappingOnLoc, false);
                        glUniform1i(m_parallaxMappingOnLoc, false);
                    }

                    /*  The mirror surface is rendered to face away to simulate the flipped effect.
                        Hence we need to perform front-face culling for it.
                        Other objects use back-face culling as usual.
                    */
                    //if (i == TO_INT(ObjID::MIRROR)) {
                    //    glCullFace(GL_FRONT);
                    //}

                    RenderObj(scene.GetObject(i));

                    /*  Trigger back-face culling again */
                    //if (i == TO_INT(ObjID::MIRROR)) {
                    //    glCullFace(GL_BACK);
                    //}
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
void Renderer::RenderToSphereCubeMapTexture(unsigned char* sphereCubeMapTexture[], Scene& scene)
{
    /*  Theoretically the rendering to cubemap texture can be done in the same way as 2D texture:
        rendering straight to the GPU cubemap texture object, similar to what we do for the
        2D mirror texture in RenderToMirrorTexture.
        However, some graphics drivers don't seem to implement the framebuffer cubemap texture properly.
        So we do the cubemap texture writing manually here: copy the framebuffer to CPU texture data,
        then copy that data to GPU texture object later (in SetUpSphereTexture function).
    */

    GLuint sphereFrameBufferID;
    glGenFramebuffers(1, &sphereFrameBufferID);
    glBindFramebuffer(GL_FRAMEBUFFER, sphereFrameBufferID);

    ResourceManager& resourceManager = ResourceManager::GetInstance();
    GLuint sphereFrameBufferTexID;
    glGenTextures(1, &sphereFrameBufferTexID);
    glBindTexture(GL_TEXTURE_2D, sphereFrameBufferTexID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, resourceManager.skyboxFaceSize, resourceManager.skyboxFaceSize,0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

    for (int i = 0; i < TO_INT(CubeFaceID::NUM_FACES); ++i)
    {
        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, sphereFrameBufferTexID, 0);

        RenderObjsBg(m_sphereCamMVMat[i], m_sphereCamNormalMVMat[i], m_sphereCamViewMat[i], m_sphereCamProjMat,
            resourceManager.skyboxFaceSize, resourceManager.skyboxFaceSize,
            RenderPass::SPHERETEX_GENERATION, scene);

        glReadBuffer(GL_COLOR_ATTACHMENT0);
        glReadPixels(0, 0, ResourceManager::GetInstance().skyboxFaceSize, ResourceManager::GetInstance().skyboxFaceSize, GL_RGBA, GL_UNSIGNED_BYTE, sphereCubeMapTexture[i]);
    }

    glDeleteTextures(1, &sphereFrameBufferTexID);
    glDeleteFramebuffers(1, &sphereFrameBufferID);
}


/******************************************************************************/
/*!
\fn     void RenderToMirrorTexture(Scene& scene)
\brief
        Render the scene to the texture for mirror reflection. This texture was
        already bound to mirrorFrameBufferID in SetUpMirrorTexture function.
*/
/******************************************************************************/
void Renderer::RenderToMirrorTexture(Scene& scene)
{
    glBindFramebuffer(GL_FRAMEBUFFER, ResourceManager::GetInstance().mirrorFrameBufferID);
    RenderObjsBg(m_mirrorCamMVMat, m_mirrorCamNormalMVMat, m_mirrorCamViewMat, m_mirrorCamProjMat,
        mirrorCam.width, mirrorCam.height,
        RenderPass::MIRRORTEX_GENERATION,scene);
}


/******************************************************************************/
/*!
\fn     void RenderToScreen(Scene& scene)
\brief
        Render the scene to the default framebuffer.
*/
/******************************************************************************/
void Renderer::RenderToScreen(Scene& scene)
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    RenderObjsBg(m_mainCamMVMat, m_mainCamNormalMVMat, m_mainCamViewMat, m_mainCamProjMat,
        mainCam.width, mainCam.height,
        RenderPass::NORMAL,scene);
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
void Renderer::Render(Scene& scene, float fps)
{
    ComputeMainCamMats(scene);
    ComputeMirrorCamMats(scene);

    /*  The texture used for sphere reflection/refraction is view-independent,
        so it only needs to be rendered once in the beginning
    */
    static bool firstFrame = true;
    if (firstFrame)
    {
        ComputeSphereCamMats(scene);
        ResourceManager& resourceManager = ResourceManager::GetInstance();
        unsigned char* sphereCubeMapData[TO_INT(CubeFaceID::NUM_FACES)];
        for (int f = 0; f < TO_INT(CubeFaceID::NUM_FACES); ++f) {
            sphereCubeMapData[f] = (unsigned char*)malloc(resourceManager.skyboxFaceSize * resourceManager.skyboxFaceSize * 4 * sizeof(unsigned char));
        }

        /*  Theoretically the rendering to cubemap texture can be done in the same way as 2D texture:
            rendering straight to the GPU texture object, similar to what we do for the
            2D mirror texture below.
            However, some graphics drivers don't seem to implement the framebuffer cubemap texture properly.
            So we do the cubemap texture generation manually here: copy the framebuffer to CPU texture data,
            then copy that data to the GPU texture object.
        */
        RenderToSphereCubeMapTexture(sphereCubeMapData, scene);
        resourceManager.SetUpSphereTexture(sphereCubeMapData);

        for (int f = 0; f < TO_INT(CubeFaceID::NUM_FACES); ++f) {
            free(sphereCubeMapData[f]);
        }

        firstFrame = false;
    }


    /*  The texture for planar reflection is view-dependent, so it needs to be rendered on the fly,
        whenever the mirror is visible and camera is moving
    */
    if (m_mirrorVisible && mainCam.moved) {
        RenderToMirrorTexture(scene);
    }

    /*  Render the scene, except the sphere to the screen */
    RenderToScreen(scene);

    /*  This is done separately, as it uses a different shader program for reflection/refraction */
    //RenderSphere(scene);


    /*  Reset */
    mainCam.moved = false;
    mainCam.resized = false;

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    RenderGui(fps);

    // Rendering    
    ImGui::Render();
    int display_w, display_h;
    glfwGetFramebufferSize(m_window.get(), &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(m_window.get());
}

Rendering::Renderer::~Renderer() {
    CleanUp(); // free all resources
}
