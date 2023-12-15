#pragma once
#include <GLFW/glfw3.h>
#include <utilities/ToUnderlyingEnum.h>
#include <rendering/Scene.h>
#include <rendering/Shader.h>
#include <unordered_map>
#include <array>
#include "Object.h"

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

	/*  We need 3 set of shaders programs */
	/*  MAIN_PROG   : Render all the objects in the scene, used for the above 3 passes */
	/*  SKYBOX_PROG : Render the background */
	/*  SPHERE_PROG : Render the relective/refractive sphere */
	enum class ProgType{
		MAIN_PROG = 0, 
		SKYBOX_PROG, 
		SPHERE_PROG, 
		NUM_PROGTYPES 
	};
	enum class ShaderType{ 
		VERTEX_SHADER = 0, 
		FRAGMENT_SHADER, 
		NUM_SHADERTYPES 
	};

	/*  For toggling the reflection/refraction of the sphere */
	enum class RefType{
		REFLECTION_ONLY = 0, 
		REFRACTION_ONLY, 
		REFLECTION_REFRACTION, 
		NUM_REFTYPES 
	};

	class Renderer {
		Scene m_scene;
		std::unordered_map<ProgType, ShaderInfo> m_shaderFileMap;  // Central map for shader file paths
		std::array <Shader, TO_INT(ProgType::NUM_PROGTYPES) > m_shaders;
									//custom deleter
		std::unique_ptr<GLFWwindow, void(*)(GLFWwindow*)> m_window;// Pointer to the window
		float m_sphereRefIndex;
		float m_fps;                  // Frame rate
		RefType m_sphereRef;          // Current reflection/refraction type for the objects
		bool m_parallaxMappingOn;     // Toggle for parallax mapping
		bool m_mirrorVisible;

		void InitImGui();
		void InitRendering();

		void EstimateFPS();
		void RenderSkybox(const Mat4& viewMat);
		void RenderObj(const Object& obj);
		void RenderSphere();
		void RenderObjsBg(const Mat4* MVMat, const Mat4* normalMVMat, const Mat4& viewMat, const Mat4& projMat, int viewportWidth, int viewportHeight, RenderPass renderPass);
		void RenderToSphereCubeMapTexture(unsigned char* sphereCubeMapTexture[]);
		void RenderToMirrorTexture();
		void RenderToScreen();
		void SendLightProperties();
		void ComputeObjMVMats(Mat4* MVMat,Mat4* NMVMat, const Mat4& viewMat);
		void ComputeMainCamMats();
		void ComputeMirrorCamMats();
		void ComputeSphereCamMats();
		void UpdateLightPosViewFrame();
		void RenderGui();

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
		void SetUpDemoScene();
		void Render();

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

