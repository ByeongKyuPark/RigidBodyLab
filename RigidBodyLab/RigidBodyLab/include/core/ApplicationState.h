#pragma once
#include <rendering/Scene.h>
#include <chrono>

using namespace Rendering;

class ApplicationState {
    //could have multiple scenes, as well. std::vector<Scene> m_scenes;
    Scene m_scene;

    /*  For displaying FPS */
    std::chrono::high_resolution_clock::time_point m_prevTime;
    std::chrono::high_resolution_clock::time_point m_currTime;
    float m_deltaTime;
    int m_frameCount;
    float m_secCount;        /*  Num of seconds from prevTime to currTime */
    float m_fps;

    //TODO
    //1. settings or preferences that might change during runtime, like graphics settings, audio levels, or control configurations.
    //2. current level, score, player health, or other gameplay-related variables.
    //3. current state of the user interface, such as which menus are open, user input, or selected options.
    //4. (done) information related to timing, like the current frame count or elapsed time, which might be useful for animations or physics updates.
    //5. information about the connection status, incoming data for Network State
    //6. logging system, a configuration manager
public:
    ApplicationState();
    float GetDeltaTime() const;
    float GetFPS()const { return m_fps; }
    Scene& getScene() { return m_scene; }
    void UpdateTime();
};
