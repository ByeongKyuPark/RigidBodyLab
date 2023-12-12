#pragma once
#include <GLFW/glfw3.h>
#include <utilities/ToUnderlyingEnum.h>
#include "Object.h"
//#include "camera.hpp"
namespace Rendering {

	/*  For displaying frame-per-second */
	//extern float fps;

	/*  For toggling the reflection/refraction of the sphere */
	enum class RefType{
		REFLECTION_ONLY = 0, 
		REFRACTION_ONLY, 
		REFLECTION_REFRACTION, 
		NUM_REFTYPES 
	};

	//extern int sphereRef;

	//extern GLFWwindow* window;

	/*  For toggling parallax mapping */
	//extern bool parallaxMappingOn;


	/*  Public functions */
	//void Init();
	//void SetUp();
	//void CleanUp();
	//void Resize(GLFWwindow* window, int width, int height);
	//void InitImGui();
	//void InitRendering(GLFWwindow* window);
	//void Render();
	//------------------------------------	

	class Renderer {
									//custom deleter
		std::unique_ptr<GLFWwindow, void(*)(GLFWwindow*)> m_window;// Pointer to the window
		float m_fps;                  // Frame rate
		bool m_parallaxMappingOn;     // Toggle for parallax mapping
		RefType m_sphereRef;          // Current reflection/refraction type for the objects

		void Init();
		void InitImGui();
		void InitRendering();

		// GLFW's window handling doesn't directly support smart pointers since the GLFW API is a C API that expects raw pointers. 
		// therefore, provided a custom deleter for the std::unique_ptr to properly handle GLFW window destruction.
		static void WindowDeleter(GLFWwindow* window) {
			glfwDestroyWindow(window);
		}

	public:
		Renderer() : m_window{ nullptr,WindowDeleter }, m_fps(0), m_parallaxMappingOn(false), m_sphereRef(RefType::REFLECTION_ONLY) {
			Init();
		}

		static Renderer& GetInstance() {
			static Renderer instance;
			return instance;
		}

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

		static void Resize(GLFWwindow* window, int width, int height);

		// Getter and setter for parallax mapping
		bool IsParallaxMappingOn() const { return m_parallaxMappingOn; }
		void SetParallaxMapping(bool on) { m_parallaxMappingOn = on; }
		bool ShouldClose()const { return glfwWindowShouldClose(m_window.get()); }
		void CleanUp();

		// Getter and setter for sphere reflection/refraction type
		RefType GetSphereRef() const { return m_sphereRef; }
		void SetSphereRef(RefType type) { m_sphereRef = type; }

	};

}

