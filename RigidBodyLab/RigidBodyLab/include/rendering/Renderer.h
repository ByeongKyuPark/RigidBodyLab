#pragma once
#include <GLFW/glfw3.h>
#include <utilities/ToUnderlyingEnum.h>
#include <rendering/Shader.h>
#include <core/Object.h>
#include <core/Scene.h>
#include <unordered_map>
#include <vector>
#include <cmath> // sin, cos
#include <array>

using Core::Scene;
using Core::Object;

namespace Rendering {

	/*  For indicating which pass we are rendering, since we will use the same shaders to render the whole scene */
	/*  SPHERETEX_GENERATION: Generating the scene texture for the sphere reflection/refraction */
	/*  MIRRORTEX_GENERATION: Generating the scene texture for the mirror reflection */
	/*  NORMAL              : Render the final scene as normal */
	enum class RenderPass {
		SPHERETEX_GENERATION = 0,
		MIRRORTEX_GENERATION,
		//FORWARD
		NORMAL,
		NUM_RENDERPASS
		//DEFERRED_LIGHT
	};

	/*  For toggling the reflection/refraction of the sphere */
	enum class RefType {
		REFLECTION_ONLY = 0,
		REFRACTION_ONLY,
		REFLECTION_REFRACTION,
		NUM_REFTYPES
	};
	/*  For activating the texture ID. We need these 3 separate IDs because
		they are used at the same time for the base
	*/
	enum class ActiveTexID {
		COLOR = 0,
		NORMAL,
		BUMP,
		//G-buffer textures
		G_ALBEDO,    // G-buffer Albedo (or diffuse) texture
		G_POSITION,  // G-buffer Position texture
		G_NORMAL,    // G-buffer Normal texture
		G_DEPTH      // G-buffer Depth texture
	};

	enum class DebugType
	{
		MAIN,
		COLOR,
		POSITION,
		NORMAL,
		DEPTH,
		NORMAL_MAPPING_MASK,// Objects for which normal mapping is not applied
		NUM_DEBUGTYPES
	};

	class ResourceManager;


	//in OpenGL, a rendering context can only be active on one thread at a time, making multi - threading complex and potentially inefficient.The sequential nature of OpenGL's state machine also means that the order of operations is crucial, and multi-threading can disrupt this order, leading to unintended consequences in rendering outcomes.	
	class Renderer {
	public:
		static constexpr int NUM_MAX_LIGHTS = 30;
	private:
		std::unordered_map<ProgType, ShaderInfo> m_shaderFileMap;  // Central map for shader file paths
		std::array <Shader, TO_INT(ProgType::NUM_PROGTYPES) > m_shaders;
		//custom deleter
		std::unique_ptr<GLFWwindow, void(*)(GLFWwindow*)> m_window;// Pointer to the window
		std::vector<int> m_guiToObjectIndexMap;

		int m_sphereMirrorCubeMapFrameCounter;
		float m_sphereRefIndex;
		float m_fps;                  // Frame rate
		RefType m_sphereRef;          // Current reflection/refraction type for the objects

		bool m_parallaxMappingOn;     // Toggle for parallax mapping
		bool m_mirrorVisible;
		bool m_shouldUpdateCubeMapForSphere;

		//deferred light
		//deferred geom

		/******************************************************************************/
		/*  Graphics-related variables                                                */
		/******************************************************************************/

		/*  Matrices for view/projetion transformations */
		/*  Viewer camera */
		Mat4 m_mainCamViewMat;
		Mat4 m_mainCamProjMat;
		std::unordered_map<int, Mat4> m_mainCamMVMat;
		std::unordered_map<int, Mat4> m_mainCamNormalMVMat;

		/*  For clearing depth buffer */
		GLfloat one = 1.0f;

		bool m_buffersDisplay = true;

		/*  Location of bump/normal textures */
		/*  For indicating whether object has normal map, and parallax mapping status */

		GLuint m_gColorTexID;
		GLuint m_gPosTexID;
		GLuint m_gNrmTexID;
		GLuint m_gDepthTexID;

		GLuint m_deferredGeomPassFBO;

		GLuint quadVAO[TO_INT(DebugType::NUM_DEBUGTYPES)];
		GLuint quadVBO[TO_INT(DebugType::NUM_DEBUGTYPES)];
		GLfloat quadBuff[TO_INT(DebugType::NUM_DEBUGTYPES)][8] =
		{
			/*  Full-screen quad, used for full-screen rendering */
			{
				-1.0f, -1.0f,
				1.0f, -1.0f,
				-1.0f, 1.0f,
				1.0f, 1.0f
			},
			/*  First bottom minimap */
			{
				-1.0f, -1.0f,
				-0.6f, -1.0f,
				-1.0f, -0.5f,
				-0.6f, -0.5f
			},
			/*  Second bottom minimap */
			{
				-0.6f, -1.0f,
				-0.2f, -1.0f,
				-0.6f, -0.5f,
				-0.2f, -0.5f
			},
			/*  Third bottom minimap */
			{
				-0.2f, -1.0f,
				0.2f, -1.0f,
				-0.2f, -0.5f,
				0.2f, -0.5f
			},
			/*  Forth bottom minimap */
			{
				0.2f, -1.0f,
				0.6f, -1.0f,
				0.2f, -0.5f,
				0.6f, -0.5f
			},
			/*  Fifth bottom minimap */
			{
				0.6f, -1.0f,
				1.0f, -1.0f,
				0.6f, -0.5f,
				1.0f, -0.5f
			}
		};

		/* (1) deferred geometry Locs */
		GLuint m_gMVMatLoc;
		GLuint m_gNMVMatLoc;
		GLuint m_gProjMatLoc;
		GLuint m_gNumLightsLoc;
		GLuint m_gAmbientLoc;
		GLuint m_gObjectTypeLoc;
		GLuint m_gNormalMappingOnLoc;
		GLuint m_gForwardRenderOnLoc;
		GLuint m_gParallaxMappingOnLoc;
		GLuint m_gColorTexLoc;
		GLuint m_gNormalTexLoc;
		GLuint m_gBumpTexLoc;
		GLuint m_gSpecularPowerLoc;
		GLuint m_gLightPosVFLoc[NUM_MAX_LIGHTS];
		GLuint m_gDiffuseLoc[NUM_MAX_LIGHTS];
		GLuint m_gSpecularLoc[NUM_MAX_LIGHTS];
		GLuint m_gLightcolorLoc[NUM_MAX_LIGHTS];
		int m_gLightPassDebug = 0;
		bool m_gBlinnPhongLighting = true;

		/* (2) deferred light Locs */
		GLuint m_lLightPassQuadLoc;
		GLuint m_lLightPassDebugLoc;
		GLuint m_lColorTexLoc;
		GLuint m_lNormalMappingObjTypeLoc;
		GLuint m_lPosTexLoc;
		GLuint m_lNrmTexLoc;
		GLuint m_lTanTexLoc;
		GLuint m_lDepthTexLoc;
		GLuint m_lNumLightsLoc;
		GLuint m_lAmbientLoc;
		GLuint m_lSpecularPowerLoc;
		GLuint m_lParallaxMappingOnLoc;
		GLuint m_lBlinnPhongLightingLoc;  // 1 for active, 0 for inactive
		GLuint m_lLightPosVFLoc[NUM_MAX_LIGHTS];
		GLuint m_lDiffuseLoc[NUM_MAX_LIGHTS];
		GLuint m_lSpecularLoc[NUM_MAX_LIGHTS];
		GLuint m_lLightcolorLoc[NUM_MAX_LIGHTS];

		//(3) sphere mirror
		GLint m_sphereMVMatLoc, m_sphereNMVMatLoc, m_sphereProjMatLoc, m_sphereViewMatLoc;  /*  used for sphere program */
		GLint m_sphereTexCubeLoc;                 /*  Texture cubemap for the sphere reflection/refraction */
		GLint m_sphereRefLoc;                     /*  For sending reflection/refraction status */
		GLint m_sphereRefIndexLoc;                     /*  For sending refractive index of the sphere */

		/*  Sphere cameras - we need 6 of them to generate the texture cubemap */
		Mat4 m_sphereCamProjMat;
		std::unordered_map<int, Mat4> m_sphereCamViewMat;
		std::unordered_map<int, std::array<Mat4, TO_INT(CubeFaceID::NUM_FACES)>> m_sphereCamMVMat;
		std::unordered_map<int, std::array<Mat4, TO_INT(CubeFaceID::NUM_FACES)>> m_sphereCamNormalMVMat;

		//(4) planar mirror
		/*  Mirror camera */
		Mat4 m_mirrorCamViewMat;
		Mat4 m_mirrorCamProjMat;
		std::unordered_map<int, Mat4> m_mirrorCamMVMat;
		std::unordered_map<int, Mat4> m_mirrorCamNormalMVMat;

		//(5) skybox
		GLint m_skyboxViewMatLoc;                             /*  used for skybox program */
		GLint m_skyboxTexCubeLoc;                 /*  Texture cubemap for the skybox background rendering */

		//(6) shadow map
		GLuint m_sLightSpaceMatLoc;
		GLuint m_shadowMapFBO;
		GLuint m_sShdowDepthTexID;

	private:
		void InitImGui();
		void InitRendering();

		void UpdateNumLights(int numLights);
		// Function to update the mapping when objects are added/removed
		void UpdateGuiToObjectIndexMap(const Core::Scene& scene);

		// Function to update light positions
		void UpdateOrbitalLights(Core::Scene& scene, float dt);

		void RenderSkybox(const Mat4& viewMat);
		void RenderObj(const Core::Object& obj);
		void RenderSphere(const Scene& scene);
		
		void RenderObjects(RenderPass renderPass, Core::Scene& scene, int faceIdx=-1);
		
		void RenderToSphereCubeMapTexture(Scene& scene);
		void RenderToMirrorTexture(Scene& scene);
		void RenderToScreen(Scene& scene);
		void RenderGui(Scene& scene, float fps);

		void ComputeMainCamObjMVMats(const Core::Scene& scene);
		void ComputePlanarMirrorCamObjMVMats(const Core::Scene& scene);
		void ComputeSphericalMirrorCamObjMVMats(int faceIdx, const Core::Scene& scene);
		void ComputeMainCamMats(const Scene& scene);
		void ComputeMirrorCamMats(const Scene& scene);
		void ComputeSphereCamMats(const Scene& scene);
		
		void SendLightColors(Scene& scene, int lightIdx=0);
		void SendDeferredLightPassProperties(const Scene& scene);
		void SendDeferredGeomProperties(const Scene& scene);
		
		void SetUpSkyBoxUniformLocations();
		void SetUpSphereUniformLocations();
		void SetUpDeferredGeomUniformLocations();
		void SetUpDeferredLightUniformLocations();
		void SetUpShadowMappingUniformLocations();

		void SetUpVertexData(Mesh& mesh);
		void SetUpShaders();
		void SetUpDeferredGeomPassTextures();
		void SetUpLightPassQuads();
		void SetUpShadowMappingTextures();

		bool ShouldUpdateSphereCubemap(float speedSqrd);
		// GLFW's window handling doesn't directly support smart pointers since the GLFW API is a C API that expects raw pointers. 
		// therefore, provided a custom deleter for the std::unique_ptr to properly handle GLFW window destruction.
		static void WindowDeleter(GLFWwindow* window);

	public:
		Renderer();

		static Renderer& GetInstance();
		static void Resize(GLFWwindow* window, int width, int height);

		//(rule of 5)
		~Renderer();
		// Copy constructor (deleted)
		Renderer(const Renderer&) = delete;
		// Copy assignment operator (deleted)
		Renderer& operator=(const Renderer&) = delete;
		// Move constructor (default)
		Renderer(Renderer&&) = default;
		// Move assignment operator (default)
		Renderer& operator=(Renderer&&) = default;

		// Methods for the Renderer class
		void AttachScene(const Scene& scene);
		void Render(Scene& scene, float fps, float dt);

		bool ShouldClose()const { return glfwWindowShouldClose(m_window.get()); }
		void CleanUp();

		// Getter and setters
		bool& GetParallaxMapping() { return m_parallaxMappingOn; }
		int GetSphereRef() const { return TO_INT(m_sphereRef); }
		GLFWwindow* GetWindow() const;

		void SetParallaxMapping(bool on) { m_parallaxMappingOn = on; }
		void SetSphereRef(RefType type) { m_sphereRef = type; }
		void SendMirrorTexID();

		bool IsParallaxMappingOn() const { return m_parallaxMappingOn; }
	};

}

