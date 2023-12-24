#pragma once
#include <GLFW/glfw3.h>
#include <utilities/ToUnderlyingEnum.h>
#include <rendering/Shader.h>
#include <core/Object.h>
#include <core/Scene.h>
#include <unordered_map>
#include <vector>
#include <rendering/Shader.h>
#include <array>

using Core::Scene;
using Core::Object;

namespace Rendering {

	/*  For indicating which pass we are rendering, since we will use the same shaders to render the whole scene */
	/*  SPHERETEX_GENERATION: Generating the scene texture for the sphere reflection/refraction */
	/*  MIRRORTEX_GENERATION: Generating the scene texture for the mirror reflection */
	/*  NORMAL              : Render the final scene as normal */
	enum class RenderPass{
		SPHERETEX_GENERATION=0, 
		MIRRORTEX_GENERATION, 
		NORMAL 
	};

	/*  For toggling the reflection/refraction of the sphere */
	enum class RefType{
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
		BUMP
	};
	
	class ResourceManager;

	//OpenGL requires that the rendering context is made current on a particular thread before calling its functions.
	// to parallelize the rendering across multiple threads
	// (option1): each thread needs its own OpenGL context 
	// (option2): need to share the context properly , which can be complex and is often not supported in ALL environments
	class Renderer {
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

		/******************************************************************************/
		/*  Graphics-related variables                                                */
		/******************************************************************************/

		/*  Matrices for view/projetion transformations */
		/*  Viewer camera */
		Mat4 m_mainCamViewMat;
		Mat4 m_mainCamProjMat;
		std::unordered_map<int, Mat4> m_mainCamMVMat;
		std::unordered_map<int, Mat4> m_mainCamNormalMVMat;

		/*  Mirror camera */
		Mat4 m_mirrorCamViewMat;
		Mat4 m_mirrorCamProjMat;
		std::unordered_map<int, Mat4> m_mirrorCamMVMat;
		std::unordered_map<int, Mat4> m_mirrorCamNormalMVMat;

		/*  Sphere cameras - we need 6 of them to generate the texture cubemap */
		Mat4 m_sphereCamProjMat;
		std::unordered_map<int, Mat4> m_sphereCamViewMat;
		std::unordered_map<int, std::array<Mat4, TO_INT(CubeFaceID::NUM_FACES)>> m_sphereCamMVMat;
		std::unordered_map<int, std::array<Mat4, TO_INT(CubeFaceID::NUM_FACES)>> m_sphereCamNormalMVMat;

		/*  For clearing depth buffer */
		GLfloat one = 1.0f;

		/*  Locations of the variables in the shader. */
		/*  Locations of transform matrices */
		GLint m_mainMVMatLoc, m_mainNMVMatLoc, m_mainProjMatLoc;  /*  used for main program */
		GLint m_skyboxViewMatLoc;                             /*  used for skybox program */
		GLint m_sphereMVMatLoc, m_sphereNMVMatLoc, m_sphereProjMatLoc, m_sphereViewMatLoc;  /*  used for sphere program */

		/*  Location of color textures */
		GLint m_textureLoc;                       /*  Normal object texture */
		GLint m_sphereTexCubeLoc;                 /*  Texture cubemap for the sphere reflection/refraction */
		GLint m_skyboxTexCubeLoc;                 /*  Texture cubemap for the skybox background rendering */

		GLint m_sphereRefLoc;                     /*  For sending reflection/refraction status */
		GLint m_sphereRefIndexLoc;                     /*  For sending refractive index of the sphere */

		/*  Location of bump/normal textures */
		GLint m_normalTexLoc, m_bumpTexLoc;

		/*  For indicating whether object has normal map, and parallax mapping status */
		GLint m_normalMappingOnLoc, m_parallaxMappingOnLoc;

		/*  Location of light data */
		GLint m_numLightsLoc, m_lightPosLoc;
		GLint m_lightOnLoc;
		GLint m_ambientLoc, m_diffuseLoc, m_specularLoc, m_specularPowerLoc;
	private:
		void InitImGui();
		void InitRendering();

		void UpdateLightPosViewFrame(Scene& scene);
		// Function to update the mapping when objects are added/removed
		void UpdateGuiToObjectIndexMap(const Core::Scene& scene);

		void RenderSkybox(const Mat4& viewMat);
		void RenderObj(const Core::Object& obj);
		void RenderSphere(const Scene& scene);
		void RenderObjsBgMainCam(RenderPass renderPass, Core::Scene& scene);
		void RenderObjsBgMirrorCam(RenderPass renderPass, Core::Scene& scene);
		void RenderObjsBgSphereCam(int faceIdx, RenderPass renderPass, Core::Scene& scene);
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

		void SendLightProperties(const Scene& scene);
		void SetUpSkyBoxUniformLocations(GLuint prog);
		void SetUpMainUniformLocations(GLuint prog);
		void SetUpSphereUniformLocations(GLuint prog); 
		void SetUpVertexData(Mesh& mesh);
		void SetUpShaders();

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
		void Render(Scene& scene, float fps);

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

