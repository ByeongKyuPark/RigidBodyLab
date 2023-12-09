#include <iostream>
#include <iomanip>

#include "GLFW/glfw3.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <rendering/Renderer.h>


void MainLoop() 
{
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // GUI code
        // ImGui::ShowDemoWindow(); 

        // Rendering
        //ImGui::Render();
        Render();
    }
}

bool InitGLFW() {
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return false;
    }

    // Set GLFW window hints for version and profile
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // Necessary for macOS compatibility
#endif

    return true;
}
//--------------------------------------------------------------------------------------------------


int main() {
    Init();
    SetUp();
    MainLoop();
    CleanUp();
    return 0;
}