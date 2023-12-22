#pragma once

#include <GLFW/glfw3.h>
#include <functional>
#include <core/Scene.h>
#include <unordered_map>

//Dependency Injection
class InputHandler {
    Core::Scene& scene;
    using KeyActionFunction = std::function<void(int key, int action)>;

public:

    InputHandler(Core::Scene& scene) : scene(scene){}
    void RegisterKeyAction(int key, KeyActionFunction action);
    void ProcessSpacebar();
private:
    std::unordered_map<int, KeyActionFunction> keyActions;
};
