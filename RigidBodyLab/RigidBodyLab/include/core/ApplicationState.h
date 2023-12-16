#include <rendering/Scene.h>

using namespace Rendering;

class ApplicationState {
    Scene m_scene;

    //TODO
    //1. settings or preferences that might change during runtime, like graphics settings, audio levels, or control configurations.
    //2. current level, score, player health, or other gameplay-related variables.
    //3. current state of the user interface, such as which menus are open, user input, or selected options.
    //4. information related to timing, like the current frame count or elapsed time, which might be useful for animations or physics updates.
    //5. information about the connection status, incoming data for Network State
    //6. logging system, a configuration manager
public:
    ApplicationState() = default;
    Scene& getScene() { return m_scene; }
};
