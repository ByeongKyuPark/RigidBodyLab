#include <core/Application.h>
#include <rendering/Renderer.h>
#include <utilities/Logger.h>
#include <memory>
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

using Rendering::Renderer;
using namespace std::chrono;

Application::Application()
    :m_scene{}, m_prevTime{},m_currTime {  }, m_frameCount{}, m_secCount{}, m_deltaTime{}, m_fps{}, m_inputHandler{ std::make_unique<InputHandler>(m_scene) }
{
    Logger::Log("Application initialized");
    Renderer::GetInstance().AttachScene(m_scene);
    Logger::Log("Scene setup completed");
}

void Application::Run() {
    Logger::Log("Starting Application Run Loop. Initializing Renderer and setting up GLFW window user pointer.");

    Renderer& renderer = Renderer::GetInstance();

    // sets a custom pointer to the GLFW window. This allows us to associate custom data 
    //(in this case, the "this" pointer of the Application instance) with the GLFW window. 
    // later, will retrieve this pointer in the GLFW callback functions to access application's state or methods.
    glfwSetWindowUserPointer(renderer.GetWindow(), this);

    // instruction loop
    while (!m_gameStarted && !renderer.ShouldClose()) {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("How to Play", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::Text("Welcome to Reflections!");
        ImGui::Text("Controls:");
        ImGui::BulletText("Use W,A,S,D keys to orbit the camera around the platform.");
        ImGui::BulletText("Press 'Space' to throw an object towards the center.");
        ImGui::BulletText("Press 'ESC' to quit the game.");

        ImGui::Text("Objective:");
        ImGui::BulletText("Stop the platform from shrinking by ensuring all remaining beings are the same type.");
        ImGui::BulletText("Secret: The key to unity might not be what you expect.");
        if (ImGui::Button("Begin Journey")) {
            m_gameStarted = true;
        }
        ImGui::End();

        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(renderer.GetWindow(), &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(renderer.GetWindow());
    }

    //hide cursor
    glfwSetInputMode(renderer.GetWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);

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

        renderer.Render(m_scene, GetFPS(), FIXED_DT);
    }
    Logger::Log("Application Run Loop ended. Renderer and GLFW window finalized.");
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
