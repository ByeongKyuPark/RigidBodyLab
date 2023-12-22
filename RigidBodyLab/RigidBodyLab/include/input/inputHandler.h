#pragma once

#include <GLFW/glfw3.h>
#include <functional>
#include <core/Scene.h>
#include <unordered_map>

class InputHandler {
    using KeyActionFunction = std::function<void(int key, int action)>;

    Core::Scene& scene;
    std::unordered_map<int, KeyActionFunction> keyActions;

    void ProcessSpacebar();
    void ProcessRKey();

    friend void Keyboard(GLFWwindow*, int, int, int, int);
public:
    InputHandler(Core::Scene& scene) : scene(scene){}
    void RegisterKeyAction(int key, KeyActionFunction action);

};
