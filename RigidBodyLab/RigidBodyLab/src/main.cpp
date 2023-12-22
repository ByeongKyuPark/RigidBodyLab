#include <iostream>
#include <iomanip>

#include "GLFW/glfw3.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <core/Application.h>
#include <rendering/Renderer.h>

using Rendering::Renderer;
using Core::Scene;



int main() {
    Application appState;
    appState.Run();
    return 0;
}