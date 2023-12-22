#include <core/Application.h>
#include <rendering/Renderer.h>
#include <memory>

using Rendering::Renderer;
using namespace std::chrono;

Application::Application()
    :m_scene{}, m_prevTime{},m_currTime {  }, m_frameCount{}, m_secCount{}, m_deltaTime{}, m_fps{}, m_inputHandler{ std::make_unique<InputHandler>(m_scene) }
{
    Renderer::GetInstance().AttachScene(m_scene);
}

void Application::Run() {
    Renderer& renderer = Renderer::GetInstance();

    // sets a custom pointer to the GLFW window. This allows us to associate custom data 
    //(in this case, the "this" pointer of the Application instance) with the GLFW window. 
    // later, will retrieve this pointer in the GLFW callback functions to access application's state or methods.
    glfwSetWindowUserPointer(renderer.GetWindow(), this);

    m_prevTime = std::chrono::high_resolution_clock::now();
    double accumulator = 0.0;

    while (!renderer.ShouldClose()) {
        glfwPollEvents();
        UpdateTime();

        accumulator += m_deltaTime;

        while (accumulator >= FIXED_DT) {
            m_scene.Update(FIXED_DT);
            accumulator -= FIXED_DT;
        }

        renderer.Render(m_scene, GetFPS());
    }
}
float Application::GetDeltaTime() const {
    return m_deltaTime;
}

void Application::UpdateTime() {
    m_currTime = std::chrono::high_resolution_clock::now();
    m_deltaTime = std::chrono::duration<double, std::chrono::seconds::period>(m_currTime - m_prevTime).count();
    m_prevTime = m_currTime;

    ++m_frameCount;
    m_secCount += m_deltaTime;

    if (m_secCount > 1.f) {
        m_fps = m_frameCount / m_secCount;
        m_frameCount = 0;
        m_secCount = 0;
    }
}
