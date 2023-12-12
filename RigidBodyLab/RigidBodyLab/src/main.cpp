#include <iostream>
#include <iomanip>

#include "GLFW/glfw3.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <rendering/Renderer.h>

using namespace Rendering;

void MainLoop(Renderer& renderer){
    while (!renderer.ShouldClose()) {
        glfwPollEvents();

        // Start the Dear ImGui frame , TODO::move
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Rendering
        renderer.Render();
    }
}

int main() {
    Renderer& renderer = Renderer::GetInstance();
    renderer.SetUpDemoScene();
    MainLoop(renderer);
    return 0;
}