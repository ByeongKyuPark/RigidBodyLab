#pragma once

void Keyboard(GLFWwindow* window, int key, int scancode, int action, int mods);

void MouseClick(GLFWwindow* window, int button, int action, int mods);

void MouseMove(GLFWwindow* window, double xpos, double ypos);

void MouseScroll(GLFWwindow* window, double xoffset, double yoffset);
