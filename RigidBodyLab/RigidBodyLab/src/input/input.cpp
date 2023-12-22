#include <rendering/Camera.h>
#include <rendering/Renderer.h>
#include <input/input.h>
#include "imgui_impl_glfw.h"

using Rendering::mainCam;

void Keyboard(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (action == GLFW_PRESS || action == GLFW_REPEAT) {
		switch (key)
		{
		case GLFW_KEY_ESCAPE:
			glfwSetWindowShouldClose(window, GLFW_TRUE); 
			break;

		case GLFW_KEY_W:
			mainCam.MoveUp();
			break;

		case GLFW_KEY_S:
			mainCam.MoveDown();
			break;

		case GLFW_KEY_A:
			mainCam.MoveLeft();
			break;

		case GLFW_KEY_D:
			mainCam.MoveRight();
			break;

		case GLFW_KEY_SPACE:
			mainCam.Reset();
			break;

		}
	}
}

void MouseClick(GLFWwindow* window, int button, int action, int mods) {
	if (action == GLFW_PRESS) {
		switch (button) {
		case GLFW_MOUSE_BUTTON_LEFT:
			// Handle left mouse button press
			break;
		case GLFW_MOUSE_BUTTON_RIGHT:
			// Handle right mouse button press
			break;
		}
	}
}


void MouseMove(GLFWwindow* window, double xpos, double ypos) {
	// xpos and ypos are the new cursor positions
}

void MouseScroll(GLFWwindow* window, double xoffset, double yoffset) {
	ImGuiIO& io = ImGui::GetIO();

	// only adjust camera zoom if the mouse is not over any ImGui window
	if (!io.WantCaptureMouse) {
		if (yoffset > 0) {
			mainCam.MoveFarther();
		}
		else if (yoffset < 0) {
			mainCam.MoveCloser();
		}
	}
}
