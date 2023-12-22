#include <input/inputHandler.h>

void InputHandler::RegisterKeyAction(int key, KeyActionFunction action) {
    keyActions[key] = action;
}

void InputHandler::ProcessKeyInput(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (keyActions.find(key) != keyActions.end()) {
        keyActions[key](key, action);
    }
}
