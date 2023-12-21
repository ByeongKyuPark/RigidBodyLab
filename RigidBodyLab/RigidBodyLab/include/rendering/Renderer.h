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

	class Renderer {
		std::unordered_map<ProgType, ShaderInfo> m_shaderFileMap;  // Central map for shader file paths
		std::array <Shader, TO_INT(ProgType::NUM_PROGTYPES) > m_shaders;
									//custom deleter
		std::unique_ptr<GLFWwindow, void(*)(GLFWwindow*)> m_window;// Pointer to the window
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
		std::vector<Mat4> m_mainCamMVMat;
		std::vector<Mat4> m_mainCamNormalMVMat;

		/*  Mirror camera */
		Mat4 m_mirrorCamViewMat;
		Mat4 m_mirrorCamProjMat;
		std::vector<Mat4> m_mirrorCamMVMat;
		std::vector<Mat4> m_mirrorCamNormalMVMat;

		/*  Sphere cameras - we need 6 of them to generate the texture cubemap */
		Mat4 m_sphereCamProjMat;
		std::vector<Mat4> m_sphereCamViewMat;
		std::vector<std::vector<Mat4>> m_sphereCamMVMat;
		std::vector<std::vector<Mat4>> m_sphereCamNormalMVMat;

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
		void InitMatrices(size_t numObjects);
		void InitImGui();
		void InitRendering();

		void UpdateLightPosViewFrame(Scene& scene);

		void RenderSkybox(const Mat4& viewMat);
		void RenderObj(const Core::Object& obj);
		void RenderSphere(const Scene& scene);
		void RenderObjsBg(const std::vector<Mat4>& MVMat, const std::vector<Mat4>& normalMVMat, const Mat4& viewMat, const Mat4& projMat, int viewportWidth, int viewportHeight, RenderPass renderPass, Core::Scene& scene);
		void RenderToSphereCubeMapTexture(unsigned char* sphereCubeMapTexture[], Scene& scene);
		void RenderToMirrorTexture(Scene& scene);
		void RenderToScreen(Scene& scene);
		void RenderGui(Scene& scene, float fps);

		void ComputeObjMVMats(std::vector<Mat4>& MVMat, std::vector<Mat4>& NMVMat, const Mat4& viewMat, const Core::Scene& scene);
		void ComputeMainCamMats(const Scene& scene);
		void ComputeMirrorCamMats(const Scene& scene);
		void ComputeSphereCamMats(const Scene& scene);

		void SendLightProperties(const Scene& scene);
		void SetUpSkyBoxUniformLocations(GLuint prog);
		void SetUpMainUniformLocations(GLuint prog);
		void SetUpSphereUniformLocations(GLuint prog); 
		void SetUpVertexData(Mesh& mesh);
		void SetUpShaders();
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

		bool IsParallaxMappingOn() const { return m_parallaxMappingOn; }
		bool& GetParallaxMapping() { return m_parallaxMappingOn; }
		void SetParallaxMapping(bool on) { m_parallaxMappingOn = on; }

		bool ShouldClose()const { return glfwWindowShouldClose(m_window.get()); }
		void CleanUp();

		// Getter and setter for sphere reflection/refraction type
		int GetSphereRef() const { return TO_INT(m_sphereRef); }
		void SetSphereRef(RefType type) { m_sphereRef = type; }
		void SendMirrorTexID();
	};

}

