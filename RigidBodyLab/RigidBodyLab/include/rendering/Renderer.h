#include <GLFW/glfw3.h>
#include "Object.h"
//#include "camera.hpp"

/*  For displaying frame-per-second */
extern float fps;

/*  For toggling the reflection/refraction of the sphere */
struct RefType
{
    enum { REFLECTION_ONLY = 0, REFRACTION_ONLY, REFLECTION_REFRACTION, NUM_REFTYPES };
};

extern int sphereRef;

extern GLFWwindow* window;

/*  For toggling parallax mapping */
extern bool parallaxMappingOn;


/*  Public functions */
void Init();
void SetUp();
void CleanUp();
void Resize(GLFWwindow* window, int width, int height);
void InitImGui();
void InitRendering(GLFWwindow* window);
void Render();

