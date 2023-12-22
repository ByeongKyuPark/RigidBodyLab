#include <core/Application.h>
#include <rendering/Renderer.h>
#include <memory>

using Rendering::Renderer;

Application::Application()
    :m_scene{}, m_currTime{  }, m_prevTime{},
    m_frameCount{}, m_secCount{}, m_deltaTime{}, m_fps{}, m_inputHandler{ std::make_unique<InputHandler>(m_scene) }
{
    Renderer::GetInstance().AttachScene(m_scene);
}

void Application::Run() {
    Renderer& renderer = Renderer::GetInstance();

    // sets a custom pointer to the GLFW window. This allows us to associate custom data 
    //(in this case, the "this" pointer of the Application instance) with the GLFW window. 
    // later, will retrieve this pointer in the GLFW callback functions to access application's state or methods.
    glfwSetWindowUserPointer(renderer.GetWindow(), this); 

    while (!renderer.ShouldClose()) {

        glfwPollEvents();
        UpdateTime();

        // 1. update the scene
        m_scene.Update(GetDeltaTime());

        // 2. render (need the FPS for GUI)
        renderer.Render(m_scene, GetFPS());
    }

}

float Application::GetDeltaTime() const {
    return m_deltaTime;
}

void Application::UpdateTime() {
    ++m_frameCount;

    m_prevTime = m_currTime;
    m_currTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float> elapsed = m_currTime - m_prevTime;
    m_deltaTime = elapsed.count();

    m_secCount += m_deltaTime;

    if (m_secCount > 1.f)
    {
        m_fps = m_frameCount / m_secCount;

        m_frameCount = 0;
        m_secCount = 0;
    }

}
