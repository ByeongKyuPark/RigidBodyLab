#pragma once

#include <GLFW/glfw3.h>
#include <functional>
#include <unordered_map>

//Dependency Injection pattern
class InputHandler {
public:
    using KeyActionFunction = std::function<void(int key, int action)>;

    void RegisterKeyAction(int key, KeyActionFunction action);
    void ProcessKeyInput(GLFWwindow* window, int key, int scancode, int action, int mods);

private:
    std::unordered_map<int, KeyActionFunction> keyActions;
};
