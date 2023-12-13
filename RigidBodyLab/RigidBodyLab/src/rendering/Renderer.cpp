#ifdef _MSC_VER
#pragma warning(push, 0)  // Microsoft Visual Studio
#define _CRT_SECURE_NO_WARNINGS
#endif

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#ifdef _MSC_VER
#pragma warning(pop)
#endif


#include <glad/glad.h>
#include <rendering/Renderer.h>
#include <rendering/Mesh.h>
#include <rendering/Camera.h>
#include <input/input.h>
#include <math/Math.h>
#include <utilities/ToUnderlyingEnum.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <ctime>

#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

using namespace Rendering;

/******************************************************************************/
/*!
\fn     ReadTextFile
\brief
        Read the content of a text file.
\param  const char fn[]
        Text filename.
\return
        The content of the given text file.
*/
/******************************************************************************/
const char* ReadTextFile(const char fn[])
{
    FILE* fp;
    char* content = NULL;
    int count = 0;

    if (fn != NULL)
    {
        fp = fopen(fn, "rt");
        if (fp != NULL)
        {
            fseek(fp, 0, SEEK_END);
            count = ftell(fp);
            rewind(fp);

            if (count > 0)
            {
                content = new char[count + 1];
                count = fread(content, sizeof(char), count, fp);
                content[count] = '\0';
            }

            fclose(fp);
        }
    }
    return content;
}


/******************************************************************************/
/*  Graphics-related variables                                                */
/******************************************************************************/

/*  For displaying FPS */
clock_t currTime, prevTime;
int frameCount;
float secCount;        /*  Num of seconds from prevTime to currTime */
float fps;

/*  For clearing depth buffer */
GLfloat one = 1.0f;


/*  For color texture */
GLuint texID[TO_INT(ImageID::NUM_IMAGES)];
const char* objTexFile[TO_INT(ImageID::NUM_IMAGES)] = { "../RigidBodyLab/images/stone_old.png", "../RigidBodyLab/images/wood.png", "../RigidBodyLab/images/pottery.jpg" };

/*  For bump/normal texture */
const char* bumpTexFile = "../RigidBodyLab/images/stone_bump.png";
GLuint bumpTexID, normalTexID;

/*  For activating the texture ID. We need these 3 separate IDs because
    they are used at the same time for the base
*/
struct ActiveTexID
{
    enum { COLOR = 0, NORMAL, BUMP };
};


/*  For environment texture */
const char* skyboxTexFile = "../RigidBodyLab/images/skybox.jpg";
GLuint skyboxTexID;
int skyboxFaceSize;

/*  6 faces of the texture cube */
enum class CubeFaceID{
    RIGHT = 0, LEFT, TOP, BOTTOM, BACK, FRONT, NUM_FACES 
};


/*  For generating sphere "reflection/refraction" texture */
GLuint sphereTexID;

/*  Toggling sphere reflection/refraction */
int sphereRef = TO_INT(RefType::REFLECTION_ONLY);


/*  For generating mirror "reflection" texture */
GLuint mirrorTexID, mirrorFrameBufferID;

/*  For turning off generating mirror "reflection" texture when mirror is not visible */
bool mirrorVisible;


/*  Toggling parallax mapping */
bool parallaxMappingOn = false;


/*  Matrices for view/projetion transformations */
/*  Viewer camera */
Mat4 mainCamViewMat, mainCamProjMat, mainCamMVMat[TO_INT(ObjID::NUM_OBJS)], mainCamNormalMVMat[TO_INT(ObjID::NUM_OBJS)];

/*  Mirror camera */
Mat4 mirrorCamViewMat, mirrorCamProjMat, mirrorCamMVMat[TO_INT(ObjID::NUM_OBJS)], mirrorCamNormalMVMat[TO_INT(ObjID::NUM_OBJS)];

/*  Sphere cameras - we need 6 of them to generate the texture cubemap */
Mat4 sphereCamViewMat[TO_INT(CubeFaceID::NUM_FACES)], 
                        sphereCamProjMat, 
                        sphereCamMVMat[TO_INT(CubeFaceID::NUM_FACES)][TO_INT(ObjID::NUM_OBJS)], 
                        sphereCamNormalMVMat[TO_INT(CubeFaceID::NUM_FACES)][TO_INT(ObjID::NUM_OBJS)];


/*  For indicating which pass we are rendering, since we will use the same shaders to render the whole scene */
/*  SPHERETEX_GENERATION: Generating the scene texture for the sphere reflection/refraction */
/*  MIRRORTEX_GENERATION: Generating the scene texture for the mirror reflection */
/*  NORMAL              : Render the final scene as normal */
struct RenderPass
{
    enum { SPHERETEX_GENERATION, MIRRORTEX_GENERATION, NORMAL };
};


/*  We need 3 set of shaders programs */
/*  MAIN_PROG   : Render all the objects in the scene, used for the above 3 passes */
/*  SKYBOX_PROG : Render the background */
/*  SPHERE_PROG : Render the relective/refractive sphere */
struct ProgType
{
    enum { MAIN_PROG = 0, SKYBOX_PROG, SPHERE_PROG, NUM_PROGTYPES };
};

struct ShaderType
{
    enum { VERTEX_SHADER = 0, FRAGMENT_SHADER, NUM_SHADERTYPES };
};

/*  Shader filenames */
const char file[ProgType::NUM_PROGTYPES][ShaderType::NUM_SHADERTYPES][100] =
{
    {
        { "../RigidBodyLab/shaders/main.vs" },
        { "../RigidBodyLab/shaders/main.fs" }
    },
    {
        { "../RigidBodyLab/shaders/skybox.vs" },
        { "../RigidBodyLab/shaders/skybox.fs" }
    },
    {
        { "../RigidBodyLab/shaders/sphere.vs" },
        { "../RigidBodyLab/shaders/sphere.fs" }
    }
};

/*  ID of the shader programs that we'll use */
GLuint prog[ProgType::NUM_PROGTYPES];



/*  Locations of the variables in the shader. */
/*  Locations of transform matrices */
GLint mainMVMatLoc, mainNMVMatLoc, mainProjMatLoc;  /*  used for main program */
GLint skyboxViewMatLoc;                             /*  used for skybox program */
GLint sphereMVMatLoc, sphereNMVMatLoc, sphereProjMatLoc, sphereViewMatLoc;  /*  used for sphere program */

/*  Location of color textures */
GLint textureLoc;                       /*  Normal object texture */
GLint sphereTexCubeLoc;                 /*  Texture cubemap for the sphere reflection/refraction */
GLint skyboxTexCubeLoc;                 /*  Texture cubemap for the skybox background rendering */

GLint sphereRefLoc;                     /*  For sending reflection/refraction status */

/*  Location of bump/normal textures */
GLint normalTexLoc, bumpTexLoc;

/*  For indicating whether object has normal map, and parallax mapping status */
GLint normalMappingOnLoc, parallaxMappingOnLoc;

/*  Location of light data */
GLint numLightsLoc, lightPosLoc;
GLint lightOnLoc;
GLint ambientLoc, diffuseLoc, specularLoc, specularPowerLoc;



/******************************************************************************/
/*!
\fn     void ValidateShader(GLuint shader, const char *file)
\brief
        Check whether shader files can be compiled successfully.
\param  shader
        ID of the shader
\param  file
        Shader file name
*/
/******************************************************************************/
void ValidateShader(GLuint shader, const char* file)
{
    const unsigned int BUFFER_SIZE = 512;
    char buffer[BUFFER_SIZE];

    GLsizei length = 0;
    GLint result;

    glGetShaderInfoLog(shader, 512, &length, buffer);
    glGetShaderiv(shader, GL_COMPILE_STATUS, &result);

    if (result != GL_TRUE && length > 0)
    {
        std::cerr << "Shader " << file << " compilation error: " << buffer << "\n";
        exit(1);
    }
    else
        std::cout << "Shader " << file << " compilation successful.\n";
}


/******************************************************************************/
/*!
\fn     void ValidateProgram(GLuint program)
\brief
        Check whether shader program can be linked successfully.
\param  program
        ID of the shader program
*/
/******************************************************************************/
void ValidateProgram(GLuint program)
{
    const unsigned int BUFFER_SIZE = 512;
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);
    GLsizei length = 0;
    GLint status;

    /*  Ask OpenGL to give us the log associated with the program */
    glGetProgramInfoLog(program, BUFFER_SIZE, &length, buffer);
    glGetProgramiv(program, GL_LINK_STATUS, &status);

    if (status != GL_TRUE && length > 0)
    {
        std::cerr << "Program " << program << " link error: " << buffer << "\n";
        exit(1);
    }
    else
        std::cout << "Program " << program << " link successful.\n";

    /*  Ask OpenGL to validate the program */
    glValidateProgram(program);
    glGetProgramiv(program, GL_VALIDATE_STATUS, &status);
    if (status == GL_FALSE)
    {
        std::cerr << "Error validating shader " << program << ".\n";
        exit(1);
    }
    else
    {
        std::cout << "Program " << program << " validation successful.\n";
    }
}


/******************************************************************************/
/*!
\fn     GLuint CompileShaders(char vsFilename[], char fsFilename[])
\brief
        Read the shader files, compile and link them into a program for render.
\param  const char vsFilename[]
        Vertex shader filename.
\param  const char fsFilename[]
        Fragment shader filename.
\return
        The rendering program ID.
*/
/******************************************************************************/
GLuint CompileShaders(const char vsFilename[], const char fsFilename[])
{
    const char* vsSource = ReadTextFile(vsFilename);
    const GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vsSource, NULL);
    glCompileShader(vertexShader);
    ValidateShader(vertexShader, vsFilename);           /*  Prints any errors */

    const char* fsSource = ReadTextFile(fsFilename);
    const GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fsSource, NULL);
    glCompileShader(fragmentShader);
    ValidateShader(fragmentShader, fsFilename);           /*  Prints any errors */

    const GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);
    ValidateProgram(program);                           /*  Print any errors */

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return program;
}


/******************************************************************************/
/*!
\fn     void SetUpSkyBoxUniformLocations(GLuint prog)
\brief
        Look up the locations of uniform variables in the skybox shader program.
\param  prog
        The given skybox shader program ID.
*/
/******************************************************************************/
void SetUpSkyBoxUniformLocations(GLuint prog)
{
    skyboxViewMatLoc = glGetUniformLocation(prog, "viewMat");
    skyboxTexCubeLoc = glGetUniformLocation(prog, "texCube");
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
void SetUpMainUniformLocations(GLuint prog)
{
    mainMVMatLoc = glGetUniformLocation(prog, "mvMat");
    mainNMVMatLoc = glGetUniformLocation(prog, "nmvMat");
    mainProjMatLoc = glGetUniformLocation(prog, "projMat");

    textureLoc = glGetUniformLocation(prog, "colorTex");

    numLightsLoc = glGetUniformLocation(prog, "numLights");
    lightPosLoc = glGetUniformLocation(prog, "lightPosVF");
    lightOnLoc = glGetUniformLocation(prog, "lightOn");

    ambientLoc = glGetUniformLocation(prog, "ambient");
    diffuseLoc = glGetUniformLocation(prog, "diffuse");
    specularLoc = glGetUniformLocation(prog, "specular");
    specularPowerLoc = glGetUniformLocation(prog, "specularPower");

    bumpTexLoc = glGetUniformLocation(prog, "bumpTex");
    normalTexLoc = glGetUniformLocation(prog, "normalTex");
    normalMappingOnLoc = glGetUniformLocation(prog, "normalMappingOn");
    parallaxMappingOnLoc = glGetUniformLocation(prog, "parallaxMappingOn");
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
void SetUpSphereUniformLocations(GLuint prog)
{
    sphereMVMatLoc = glGetUniformLocation(prog, "mvMat");
    sphereNMVMatLoc = glGetUniformLocation(prog, "nmvMat");
    sphereProjMatLoc = glGetUniformLocation(prog, "projMat");
    sphereViewMatLoc = glGetUniformLocation(prog, "viewMat");

    sphereTexCubeLoc = glGetUniformLocation(prog, "texCube");
    sphereRefLoc = glGetUniformLocation(prog, "sphereRef");
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
void SetUpVertexData(Mesh& mesh)
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
\fn     void UpdateLightPosViewFrame()
\brief
Compute view-frame light positions and send them to shader when needed.
*/
/******************************************************************************/
void Renderer::UpdateLightPosViewFrame()
{
    if (mainCam.moved)
    {
        for (int i = 0; i < m_scene.NUM_LIGHTS; ++i) {
            m_scene.m_lightPosVF[i] = Vec3(mainCamViewMat * Vec4(m_scene.m_lightPosWF[i], 1.0f));
        }

        glUniform3fv(lightPosLoc, m_scene.NUM_LIGHTS, ValuePtr(m_scene.m_lightPosVF[0]));
    }
}
/******************************************************************************/
/*!
\fn     void SendLightProperties()
\brief
        Send numLights and intensities to the rendering program.
*/
/******************************************************************************/
void Renderer::SendLightProperties()
{
    glUniform1i(numLightsLoc, m_scene.NUM_LIGHTS);

    /*  ambient, diffuse, specular are now reflected components on the object
        surface and can be used directly as intensities in the lighting equation.
    */
    Vec4 ambient, diffuse, specular;
    ambient = m_scene.m_I * m_scene.m_ambientAlbedo;
    diffuse = m_scene.m_I * m_scene.m_diffuseAlbedo;
    specular = m_scene.m_I * m_scene.m_specularAlbedo;

    glUniform4fv(ambientLoc, 1, ValuePtr(ambient));
    glUniform4fv(diffuseLoc, 1, ValuePtr(diffuse));
    glUniform4fv(specularLoc, 1, ValuePtr(specular));

    glUniform1i(specularPowerLoc, m_scene.m_specularPower);
}


/******************************************************************************/
/*!
\fn     ComputeObjMVMats(Mat4 MVMat[], Mat4 NMVMat[], Mat4 viewMat)
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
void Renderer::ComputeObjMVMats(Mat4 MVMat[], Mat4 NMVMat[], Mat4 viewMat)
{
    const size_t OBJ_SIZE = TO_INT(ObjID::NUM_OBJS);
    for (int i = 0; i < OBJ_SIZE; ++i)
    {
        MVMat[i] = viewMat * m_scene.m_objects[i].GetModelMatrix();
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
void Renderer::ComputeMainCamMats()
{
    /*  Update view transform matrix */
    if (mainCam.moved)
    {
        mainCamViewMat = mainCam.ViewMat();
        ComputeObjMVMats(mainCamMVMat, mainCamNormalMVMat, mainCamViewMat);
    }

    /*  Update projection matrix */
    if (mainCam.resized)
        mainCamProjMat = mainCam.ProjMat();
}


/******************************************************************************/
/*!
\fn     void ComputeMirrorCamMats()
\brief
        Compute the view/projection and other related matrices for mirror camera.
*/
/******************************************************************************/
void Renderer::ComputeMirrorCamMats()
{
    if (mainCam.moved)
    {
        /*  Computing position of user camera in mirror frame */
        Vec3 mainCamMirrorFrame = Vec3(Rotate(-m_scene.m_mirrorRotationAngle, m_scene.m_mirrorRotationAxis) 
                                        *Translate(-m_scene.m_mirrorTranslate) * Vec4(mainCam.pos, 1.0));

        /*  If user camera is behind mirror, then mirror is not visible and no need to compute anything */
        if (mainCamMirrorFrame.z <= 0)
        {
            mirrorVisible = false;
            return;
        }
        else
            mirrorVisible = true;

        /*  In mirror frame, mirror camera position is defined as (x, y, -z) in which (x, y, z) is the
            user camera position in mirror frame.
            We also need to compute mirrorCam.pos, mirrorCam.upVec, mirrorCam.lookAt are defined in world
            frame to compute mirror cam's view matrix.
            function to compute mirrorCamViewMat
        */

        Vec3 mirrorCamMirrorFrame = Vec3(mainCamMirrorFrame.x, mainCamMirrorFrame.y, -mainCamMirrorFrame.z);

        mirrorCam.pos = Vec3(Translate(m_scene.m_mirrorTranslate) * Rotate(m_scene.m_mirrorRotationAngle, m_scene.m_mirrorRotationAxis) * Vec4(mirrorCamMirrorFrame, 1.0));
        mirrorCam.upVec = BASIS[Y];
        mirrorCam.lookAt = Vec3(Translate(m_scene.m_mirrorTranslate) * Rotate(m_scene.m_mirrorRotationAngle, m_scene.m_mirrorRotationAxis) * Vec4(0, 0, 0, 1));

        mirrorCamViewMat = LookAt(mirrorCam.pos, mirrorCam.lookAt, mirrorCam.upVec);

        ComputeObjMVMats(mirrorCamMVMat, mirrorCamNormalMVMat, mirrorCamViewMat);
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
            Vec3 midCamFrame = Vec3(Translate(m_scene.m_mirrorTranslate) * Rotate(m_scene.m_mirrorRotationAngle, m_scene.m_mirrorRotationAxis) * Vec4(midPoint, 1.0));//mid : mirrorFrame -> cameraFrame
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
        mirrorCamProjMat = mirrorCam.ProjMat();
    }
}


/******************************************************************************/
/*!
\fn     void ComputeSphereCamMats()
\brief
        Compute the view/projection and other related matrices for sphere camera.
*/
/******************************************************************************/
void Renderer::ComputeSphereCamMats()
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
        sphereCamViewMat[f] = LookAt(m_scene.m_spherePos, m_scene.m_spherePos + lookAt[f], upVec[f]);
        ComputeObjMVMats(sphereCamMVMat[f], sphereCamNormalMVMat[f], sphereCamViewMat[f]);
    }


    /*  Use Perspective function to ompute the projection matrix sphereCamProjMat so that
        from the camera position at the cube center, we see a complete face of the cube.
        The near plane distance is 0.01f. The far plane distance is equal to mainCam's farPlane.
    */
    // Compute the projection matrix for the sphere camera
    float fov = PI / 2.f; // 90 degrees in radians
    float aspectRatio = 1.0f;
    float nearPlane = 0.01f; // near plane is 0.01, and far plane is the same as the main camera's far plane
    sphereCamProjMat = Perspective(fov, aspectRatio, nearPlane, mainCam.farPlane);
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
\fn     void SetUpObjTextures()
\brief
        Read texture images from files, then copy them to graphics memory.
        These textures will be combined with light colors for the objects
        in the scene.
*/
/******************************************************************************/
void SetUpObjTextures()
{
    glGenTextures(TO_INT(ImageID::NUM_IMAGES), texID);

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
            glBindTexture(GL_TEXTURE_2D, texID[i]);

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
void Bump2Normal(const unsigned char* bumpImg, unsigned char* normalImg, int width, int height)
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


/******************************************************************************/
/*!
\fn     void SetUpBaseBumpNormalTextures()
\brief
        Set up the bump map and normal map for normal mapping and parallax mapping.
*/
/******************************************************************************/
void SetUpBaseBumpNormalTextures()
{
    unsigned char* bumpImgData, * normalImgData;
    int imgWidth, imgHeight, numComponents;

    /*  Load bump image */
    bumpImgData = stbi_load(bumpTexFile, &imgWidth, &imgHeight, &numComponents,0);
    if (!bumpImgData)
    {
        std::cerr << "Reading " << bumpTexFile << " failed.\n";
        exit(1);
    }

    /*  Create normal image */
    normalImgData = (unsigned char*)malloc(imgWidth * imgHeight * 3 * sizeof(unsigned char));

    Bump2Normal(bumpImgData, normalImgData, imgWidth, imgHeight);
    /*  Can use this to test normal image */
    //SaveImageFile("stone_normal.png", normalImgData, imgWidth, imgHeight, 3);


    /*  Generate texture ID for bump image and copy it to GPU */
    /*  Bump image will be used to compute the offset in parallax mapping */
    glGenTextures(1, &bumpTexID);
    glBindTexture(GL_TEXTURE_2D, bumpTexID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, imgWidth, imgHeight, 0, GL_RED, GL_UNSIGNED_BYTE, bumpImgData);

    stbi_image_free(bumpImgData);

    /*  Generate texture mipmaps. */
    glGenerateMipmap(GL_TEXTURE_2D);
    /*  Set up texture behaviors */
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);


    /*  Generate texture ID for normal image and copy it to GPU */
    glGenTextures(1, &normalTexID);
    glBindTexture(GL_TEXTURE_2D, normalTexID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, imgWidth, imgHeight, 0,
        GL_RGB, GL_UNSIGNED_BYTE, normalImgData);

    stbi_image_free(normalImgData);

    /*  Generate texture mipmaps. */
    glGenerateMipmap(GL_TEXTURE_2D);

    /*  Set up texture behaviors */
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
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
void CopySubTexture(unsigned char* destTex, const unsigned char* srcTex,
    int size, int imgWidth,
    int verticalOffset, int horizontalOffset,
    bool verticalFlip, bool horizontalFlip,
    int numComponents)
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
\fn     void SetUpSkyBoxTexture()
\brief
        Set up the cubemap texture from the skybox image.
*/
/******************************************************************************/
void SetUpSkyBoxTexture()
{
    unsigned char* cubeImgData, * cubeFace[TO_INT(CubeFaceID::NUM_FACES)];
    int imgWidth, imgHeight, numComponents;

    cubeImgData = stbi_load(skyboxTexFile, &imgWidth, &imgHeight, &numComponents, 0);
    if (!cubeImgData) {
        std::cerr << "Reading " << skyboxTexFile << " failed.\n";
        exit(1);
    }

    skyboxFaceSize = imgHeight / 3;

    int imgSizeBytes = sizeof(unsigned char) * skyboxFaceSize * skyboxFaceSize * numComponents;

    for (int f = 0; f < TO_INT(CubeFaceID::NUM_FACES); ++f) {
        cubeFace[f] = (unsigned char*)malloc(imgSizeBytes);
    }


    /*  Copy the texture from the skybox image to 6 textures using CopySubTexture */
    /*  imgWidth is the width of the original image, while skyboxFaceSize is the size of each face */
    /*  The cubemap layout is as described in the assignment specs */
    CopySubTexture(cubeFace[TO_INT(CubeFaceID::FRONT)], cubeImgData, skyboxFaceSize, imgWidth, skyboxFaceSize, skyboxFaceSize, true, true, numComponents);
    CopySubTexture(cubeFace[TO_INT(CubeFaceID::BOTTOM)], cubeImgData, skyboxFaceSize, imgWidth, skyboxFaceSize, 0, false, false, numComponents);
    CopySubTexture(cubeFace[TO_INT(CubeFaceID::LEFT)], cubeImgData, skyboxFaceSize, imgWidth, 0, skyboxFaceSize, true, true, numComponents);
    CopySubTexture(cubeFace[TO_INT(CubeFaceID::RIGHT)], cubeImgData, skyboxFaceSize, imgWidth, 2 * skyboxFaceSize, skyboxFaceSize, true, true, numComponents);
    CopySubTexture(cubeFace[TO_INT(CubeFaceID::TOP)], cubeImgData, skyboxFaceSize, imgWidth, skyboxFaceSize, 2 * skyboxFaceSize, false, false, numComponents);
    CopySubTexture(cubeFace[TO_INT(CubeFaceID::BACK)], cubeImgData, skyboxFaceSize, imgWidth, 3 * skyboxFaceSize, skyboxFaceSize, true, true, numComponents);

    glGenTextures(1, &skyboxTexID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexID);


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
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + f, 0, internalFormat, skyboxFaceSize, skyboxFaceSize, 0, format, GL_UNSIGNED_BYTE, cubeFace[f]);
    }

    // Set the texture parameters
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    // Generate mipmaps for the cubemap
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    stbi_image_free(cubeImgData);

    for (int f = 0; f < TO_INT(CubeFaceID::NUM_FACES); ++f)
        free(cubeFace[f]);
}


/******************************************************************************/
/*!
\fn     void SetUpMirrorTexture()
\brief
        Set up texture and frame buffer objects for rendering mirror reflection.
*/
/******************************************************************************/
void SetUpMirrorTexture()
{
    glGenTextures(1, &mirrorTexID);
    glBindTexture(GL_TEXTURE_2D, mirrorTexID);

    /*  Some graphics drivers don't support glTexStorage2D */
    //glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, mirrorCam.width, mirrorCam.height);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, mirrorCam.width, mirrorCam.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

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

    glGenFramebuffers(1, &mirrorFrameBufferID);
    glBindFramebuffer(GL_FRAMEBUFFER, mirrorFrameBufferID);

    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, mirrorTexID, 0);
}


/******************************************************************************/
/*!
\fn     void SetUpSphereTexture(unsigned char *sphereCubeMapData[])
\brief
        Set up texture object for rendering sphere reflection/refraction.
*/
/******************************************************************************/
void SetUpSphereTexture(unsigned char* sphereCubeMapData[])
{
    glGenTextures(1, &sphereTexID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, sphereTexID);

    for (int f = 0; f < TO_INT(CubeFaceID::NUM_FACES); ++f)
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + f, 0, GL_RGBA8, skyboxFaceSize, skyboxFaceSize,
            0, GL_RGBA, GL_UNSIGNED_BYTE, sphereCubeMapData[f]);

    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
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
void SendMirrorTexID()
{
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, mirrorTexID);
    glUniform1i(textureLoc, 0);
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
void Renderer::SetUpDemoScene()
{
    /*  Initialization for fps estimation */
    currTime = clock();
    frameCount = 0;
    secCount = 0;

    /*  Set Up Scene:
        - Obj attributes, which include mesh type, size, pos, color/texture ...
        - Light positions
    */
    //SetUpScene();

    for (int i = 0; i < ProgType::NUM_PROGTYPES; ++i)
        prog[i] = CompileShaders(file[i][ShaderType::VERTEX_SHADER], file[i][ShaderType::FRAGMENT_SHADER]);


    /*  Send mesh data only */
    size_t NUM_MESHES = TO_INT(MeshID::NUM_MESHES);
    for (int i = 0; i < NUM_MESHES; ++i) {
        Mesh& mesh = m_scene.GetResourceManager().GetMesh(static_cast<MeshID>(i));
        SetUpVertexData(mesh);
    }

    /*  Set up textures for objects in the scene */
    SetUpObjTextures();

    /*  Set up bump map and normal map for the base object */
    SetUpBaseBumpNormalTextures();

    /*  Set up skybox texture for background rendering */
    SetUpSkyBoxTexture();

    /*  Set up texture object for mirror reflection. This texture object hasn't stored any data yet.
        We will render the reflected data for this texture on the fly.
    */
    SetUpMirrorTexture();


    /*  Look up for the locations of the uniform variables in the shader programs */
    glUseProgram(prog[ProgType::SKYBOX_PROG]);
    SetUpSkyBoxUniformLocations(prog[ProgType::SKYBOX_PROG]);

    glUseProgram(prog[ProgType::SPHERE_PROG]);
    SetUpSphereUniformLocations(prog[ProgType::SPHERE_PROG]);

    glUseProgram(prog[ProgType::MAIN_PROG]);
    SetUpMainUniformLocations(prog[ProgType::MAIN_PROG]);


    /*  Send light info to the programs.
        Vertex data and textures are copied to the graphics memory, which are mapped to the
        corresponding variables in shaders when needed. Hence, no program needs to be specified yet.
        However, uniform light data is sent directly to shaders, hence we have to indicate the
        programs to be sent to.
    */
    SendLightProperties();


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

    size_t NUM_MESHES = TO_INT(MeshID::NUM_MESHES);
    for (int i = 0; i < NUM_MESHES; ++i)
    {
        Mesh& mesh = m_scene.GetResourceManager().GetMesh(static_cast<MeshID>(i));
        glDeleteVertexArrays(1, &mesh.VAO);
        glDeleteBuffers(1, &mesh.VBO);
        glDeleteBuffers(1, &mesh.IBO);
    }

    glDeleteTextures(TO_INT(ImageID::NUM_IMAGES), texID);
    glDeleteTextures(1, &bumpTexID);
    glDeleteTextures(1, &normalTexID);
    glDeleteTextures(1, &skyboxTexID);
    glDeleteTextures(1, &mirrorTexID);
    glDeleteTextures(1, &sphereTexID);

    glDeleteFramebuffers(1, &mirrorFrameBufferID);

    //glDeleteProgram(renderProg);
    //for (int y = 0; y < ProgType::NUM_PROGTYPES; ++y)
    for (int i = 0; i < TO_INT(ProgType::NUM_PROGTYPES); ++i) {
        glDeleteProgram(prog[i]);
    }

    // Cleanup GLFW
    // Note: glfwDestroyWindow call is removed as it's handled by std::unique_ptr
    //glfwDestroyWindow(m_window);

    // Note: glfwTerminate call can remain here if this is the designated point for terminating GLFW
    //glfwTerminate();
}

Rendering::Renderer::Renderer()
    : m_window{ nullptr,WindowDeleter }, m_fps(0), m_parallaxMappingOn(true), m_sphereRef(RefType::REFLECTION_ONLY)
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
	GLFWwindow* rawWindow = glfwCreateWindow(DISPLAY_SIZE, DISPLAY_SIZE, "RigidBodyLab", nullptr, nullptr);
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
	stbi_set_flip_vertically_on_load(true);
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

// GLFW's window handling doesn't directly support smart pointers since the GLFW API is a C API that expects raw pointers. 
// therefore, provided a custom deleter for the std::unique_ptr to properly handle GLFW window destruction.

void Rendering::Renderer::WindowDeleter(GLFWwindow* window) {
    glfwDestroyWindow(window);
}

/******************************************************************************/
/*!
\fn     void EstimateFPS()
\brief
        Estimating FPS. This only updates the FPS about once per second.
*/
/******************************************************************************/
void Renderer::EstimateFPS()
{
    ++frameCount;

    prevTime = currTime;
    currTime = clock();
    secCount += 1.0f * (currTime - prevTime) / CLOCKS_PER_SEC;

    if (secCount > 1.0f)
    {
        fps = frameCount / secCount;

        frameCount = 0;
        secCount = 0;
    }
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

    glUseProgram(prog[ProgType::SKYBOX_PROG]);

    SendCubeTexID(skyboxTexID, skyboxTexCubeLoc);
    SendViewMat(viewMat, skyboxViewMatLoc);

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
void Renderer::RenderObj(const Object& obj)
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
void Renderer::RenderSphere()
{
    glUseProgram(prog[ProgType::SPHERE_PROG]);

    SendCubeTexID(sphereTexID, sphereTexCubeLoc);

    /*  Indicate whether we want reflection/refraction or both */
    glUniform1i(sphereRefLoc, sphereRef);

    /*  We need view mat to know our camera orientation */
    SendViewMat(mainCamViewMat, sphereViewMatLoc);

    /*  These are for transforming vertices on the sphere */
    SendMVMat(mainCamMVMat[TO_INT(ObjID::SPHERE)], mainCamNormalMVMat[TO_INT(ObjID::SPHERE)], sphereMVMatLoc, sphereNMVMatLoc);
    SendProjMat(mainCamProjMat, sphereProjMatLoc);

    RenderObj(m_scene.GetObject(TO_INT(ObjID::SPHERE)));
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
void Renderer::RenderObjsBg(Mat4 MVMat[], Mat4 normalMVMat[], Mat4 viewMat, Mat4 projMat,
    int viewportWidth, int viewportHeight,
    int renderPass)
{
    /*  We need to set this here because the onscreen rendering will use a bigger viewport than
        the rendering of sphere/mirror reflection/refraction texture
    */
    glViewport(0, 0, viewportWidth, viewportHeight);

    RenderSkybox(viewMat);

    glUseProgram(prog[ProgType::MAIN_PROG]);
    UpdateLightPosViewFrame();
    SendProjMat(projMat, mainProjMatLoc);

    /*  Send object texture and render them */
    size_t NUM_OBJS = TO_INT(ObjID::NUM_OBJS);
    for (int i = 0; i < NUM_OBJS; ++i)
        if (i == TO_INT(ObjID::SPHERE)) {
            continue;           /*  Will use sphere rendering program to apply reflection & refraction textures on sphere */
        }
        else {
            if (renderPass == RenderPass::MIRRORTEX_GENERATION
                && (i == TO_INT(ObjID::MIRROR) || i == TO_INT(ObjID::MIRRORBASE1)
                    || i == TO_INT(ObjID::MIRRORBASE2) || i == TO_INT(ObjID::MIRRORBASE3))) 
            {
                continue;           /*  Not drawing objects behind mirror & mirror itself */
            }
            else {
                if (renderPass == RenderPass::SPHERETEX_GENERATION && (i == TO_INT(ObjID::MIRROR))) {
                    continue;           /*  Not drawing mirror when generating reflection/refraction texture for sphere to avoid inter-reflection */
                }
                else
                {
                    if (i == TO_INT(ObjID::MIRROR))
                    {
                        SendMirrorTexID();
                        glUniform1i(lightOnLoc, 0);     /*  disable lighting on mirror surface */
                    }
                    else
                    {
                        SendObjTexID(texID[TO_INT(m_scene.GetObject(i).GetImageID())], ActiveTexID::COLOR, textureLoc);
                        glUniform1i(lightOnLoc, 1);     /*  enable lighting for other objects */
                    }

                    SendMVMat(MVMat[i], normalMVMat[i], mainMVMatLoc, mainNMVMatLoc);

                    if (i == TO_INT(ObjID::BASE))   /*  apply normal mapping / parallax mapping for the base */
                    {
                        SendObjTexID(normalTexID, ActiveTexID::NORMAL, normalTexLoc);
                        glUniform1i(normalMappingOnLoc, true);
                        glUniform1i(parallaxMappingOnLoc, parallaxMappingOn);

                        if (parallaxMappingOn)
                            SendObjTexID(bumpTexID, ActiveTexID::BUMP, bumpTexLoc);
                    }
                    else                       /*  not apply normal mapping / parallax mapping for other objects */
                    {
                        glUniform1i(normalMappingOnLoc, false);
                        glUniform1i(parallaxMappingOnLoc, false);
                    }

                    /*  The mirror surface is rendered to face away to simulate the flipped effect.
                        Hence we need to perform front-face culling for it.
                        Other objects use back-face culling as usual.
                    */
                    if (i == TO_INT(ObjID::MIRROR)) {
                        glCullFace(GL_FRONT);
                    }

                    RenderObj(m_scene.GetObject(i));

                    /*  Trigger back-face culling again */
                    if (i == TO_INT(ObjID::MIRROR)) {
                        glCullFace(GL_BACK);
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
void Renderer::RenderToSphereCubeMapTexture(unsigned char* sphereCubeMapTexture[])
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

    GLuint sphereFrameBufferTexID;
    glGenTextures(1, &sphereFrameBufferTexID);
    glBindTexture(GL_TEXTURE_2D, sphereFrameBufferTexID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, skyboxFaceSize, skyboxFaceSize,
        0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

    for (int i = 0; i < TO_INT(CubeFaceID::NUM_FACES); ++i)
    {
        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, sphereFrameBufferTexID, 0);

        RenderObjsBg(sphereCamMVMat[i], sphereCamNormalMVMat[i], sphereCamViewMat[i], sphereCamProjMat,
            skyboxFaceSize, skyboxFaceSize,
            RenderPass::SPHERETEX_GENERATION);

        glReadBuffer(GL_COLOR_ATTACHMENT0);
        glReadPixels(0, 0, skyboxFaceSize, skyboxFaceSize, GL_RGBA, GL_UNSIGNED_BYTE, sphereCubeMapTexture[i]);
    }

    glDeleteTextures(1, &sphereFrameBufferTexID);
    glDeleteFramebuffers(1, &sphereFrameBufferID);
}


/******************************************************************************/
/*!
\fn     void RenderToMirrorTexture()
\brief
        Render the scene to the texture for mirror reflection. This texture was
        already bound to mirrorFrameBufferID in SetUpMirrorTexture function.
*/
/******************************************************************************/
void Renderer::RenderToMirrorTexture()
{
    glBindFramebuffer(GL_FRAMEBUFFER, mirrorFrameBufferID);
    RenderObjsBg(mirrorCamMVMat, mirrorCamNormalMVMat, mirrorCamViewMat, mirrorCamProjMat,
        mirrorCam.width, mirrorCam.height,
        RenderPass::MIRRORTEX_GENERATION);
}


/******************************************************************************/
/*!
\fn     void RenderToScreen()
\brief
        Render the scene to the default framebuffer.
*/
/******************************************************************************/
void Renderer::RenderToScreen()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    RenderObjsBg(mainCamMVMat, mainCamNormalMVMat, mainCamViewMat, mainCamProjMat,
        mainCam.width, mainCam.height,
        RenderPass::NORMAL);
}




bool firstFrame = true;

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
void Renderer::Render()
{
    ComputeMainCamMats();
    ComputeMirrorCamMats();

    /*  The texture used for sphere reflection/refraction is view-independent,
        so it only needs to be rendered once in the beginning
    */
    if (firstFrame)
    {
        ComputeSphereCamMats();

        unsigned char* sphereCubeMapData[TO_INT(CubeFaceID::NUM_FACES)];
        for (int f = 0; f < TO_INT(CubeFaceID::NUM_FACES); ++f) {
            sphereCubeMapData[f] = (unsigned char*)malloc(skyboxFaceSize * skyboxFaceSize * 4 * sizeof(unsigned char));
        }

        /*  Theoretically the rendering to cubemap texture can be done in the same way as 2D texture:
            rendering straight to the GPU texture object, similar to what we do for the
            2D mirror texture below.
            However, some graphics drivers don't seem to implement the framebuffer cubemap texture properly.
            So we do the cubemap texture generation manually here: copy the framebuffer to CPU texture data,
            then copy that data to the GPU texture object.
        */
        RenderToSphereCubeMapTexture(sphereCubeMapData);
        SetUpSphereTexture(sphereCubeMapData);

        for (int f = 0; f < TO_INT(CubeFaceID::NUM_FACES); ++f)
            free(sphereCubeMapData[f]);

        firstFrame = false;
    }


    /*  The texture for planar reflection is view-dependent, so it needs to be rendered on the fly,
        whenever the mirror is visible and camera is moving
    */
    if (mirrorVisible && mainCam.moved)
        RenderToMirrorTexture();

    /*  Render the scene, except the sphere to the screen */
    RenderToScreen();

    /*  This is done separately, as it uses a different shader program for reflection/refraction */
    RenderSphere();


    /*  Reset */
    mainCam.moved = false;
    mainCam.resized = false;

    EstimateFPS();

    // Displaying FPS
    ImGui::SetNextWindowPos(ImVec2(DISPLAY_SIZE, 0));
    ImGui::SetNextWindowSize(ImVec2(GUI_WIDTH, GUI_WIDTH * 2.f));    ImGui::Text("Frame Rate: %.1f", fps); // Assuming fps is a float variable
    // Sphere reflection & refraction
    const char* refTypes[] = { "Reflection Only", "Refraction Only", "Reflection & Refraction" };
    ImGui::Combo("Sphere", &sphereRef, refTypes, IM_ARRAYSIZE(refTypes)); // Assuming sphereRef is an int variable
    // Parallax mapping toggling
    ImGui::Checkbox("Parallax Mapping", &parallaxMappingOn); // Assuming parallaxMappingOn is a bool variable

    // Rendering
    ImGui::Render();
    int display_w, display_h;
    glfwGetFramebufferSize(m_window.get(), &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    //glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
    //glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(m_window.get());
}

Rendering::Renderer::~Renderer() {
    CleanUp(); // free all resources
    glfwTerminate(); // terminate GLFW after all resources are released
}
