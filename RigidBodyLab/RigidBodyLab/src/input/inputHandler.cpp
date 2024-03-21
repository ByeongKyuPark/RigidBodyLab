#include <input/inputHandler.h>
#include <rendering/Camera.h>
#include <rendering/Renderer.h>
using Rendering::mainCam;

void InputHandler::RegisterKeyAction(int key, KeyActionFunction action) {
    keyActions[key] = action;
}
void InputHandler::ProcessSpacebar() {
    scene.ShootProjectile({ mainCam.GetPos().x, mainCam.GetPos().y, mainCam.GetPos().z });
}
void InputHandler::ProcessRKey() {
    Rendering::Renderer::GetInstance().Reset();
    scene.Reset();
}