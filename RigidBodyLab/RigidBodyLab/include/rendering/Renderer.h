#pragma once
#include <GLFW/glfw3.h>
#include <utilities/ToUnderlyingEnum.h>
#include <rendering/Scene.h>
#include "Object.h"
namespace Rendering {

	/*  For toggling the reflection/refraction of the sphere */
	enum class RefType{
		REFLECTION_ONLY = 0, 
		REFRACTION_ONLY, 
		REFLECTION_REFRACTION, 
		NUM_REFTYPES 
	};

	class Renderer {
									//custom deleter
		std::unique_ptr<GLFWwindow, void(*)(GLFWwindow*)> m_window;// Pointer to the window
		float m_fps;                  // Frame rate
		bool m_parallaxMappingOn;     // Toggle for parallax mapping
		RefType m_sphereRef;          // Current reflection/refraction type for the objects
		Scene m_scene;

		void InitImGui();
		void InitRendering();

		void EstimateFPS();
		void RenderSkybox(const Mat4& viewMat);
		void RenderObj(const Object& obj);
		void RenderSphere();
		void RenderObjsBg(Mat4 MVMat[], Mat4 normalMVMat[], Mat4 viewMat, Mat4 projMat, int viewportWidth, int viewportHeight, int renderPass);
		void RenderToSphereCubeMapTexture(unsigned char* sphereCubeMapTexture[]);
		void RenderToMirrorTexture();
		void RenderToScreen();
		void SendLightProperties();
		void ComputeObjMVMats(Mat4 MVMat[], Mat4 NMVMat[], Mat4 viewMat);
		void ComputeMainCamMats();
		void ComputeMirrorCamMats();
		void ComputeSphereCamMats();
		void UpdateLightPosViewFrame();
		
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
		RefType GetSphereRef() const { return m_sphereRef; }
		void SetSphereRef(RefType type) { m_sphereRef = type; }

	};

}

