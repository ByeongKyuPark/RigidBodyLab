#include <iostream>
#include <iomanip>

#include "GLFW/glfw3.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <core/ApplicationState.h>
#include <rendering/Renderer.h>

using namespace Rendering;

void MainLoop(ApplicationState& appState){
    Renderer& renderer = Renderer::GetInstance();
    while (!renderer.ShouldClose()) {
        glfwPollEvents();

        appState.UpdateTime();
        float deltaTime = appState.GetDeltaTime();
        // 1. physics update
        //appState.getScene().updatePhysics(deltaTime);

        // 2. render
        renderer.Render(appState.getScene(),appState.GetFPS());
    }
}

int main() {
    ApplicationState appState;
    Renderer::GetInstance().AttachScene(appState.getScene());
    MainLoop(appState);
    return 0;
}