#pragma once
#include <core/Scene.h>
#include <chrono>
#include <input/InputHandler.h>
class Application {
    Core::Scene m_scene;
    std::unique_ptr<InputHandler> m_inputHandler;
    bool m_gameStarted{ false };

    /*  For displaying FPS */
    std::chrono::high_resolution_clock::time_point m_prevTime;
    std::chrono::high_resolution_clock::time_point m_currTime;
    float m_deltaTime;
    int m_frameCount;
    float m_secCount;        /*  Num of seconds from prevTime to currTime */
    float m_fps;
    static constexpr float FIXED_DT= 1/120.f;

    //TODO
    //1. settings or preferences that might change during runtime, like graphics settings, audio levels, or control configurations.
    //2. current level, score, player health, or other gameplay-related variables.
    //3. current state of the user interface, such as which menus are open, user input, or selected options.
    //4. (done) information related to timing, like the current frame count or elapsed time, which might be useful for animations or physics updates.
    //5. information about the connection status, incoming data for Network State
    //6. logging system, a configuration manager
    void UpdateTime();
    float GetDeltaTime() const;
    float GetFPS()const { return m_fps; }
public:
    Application();

    InputHandler& GetInputHandler() { return *m_inputHandler; }
    void ProcessInput() {
    }
    void Run();
};
