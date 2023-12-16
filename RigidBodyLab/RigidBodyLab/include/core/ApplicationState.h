#include <rendering/Scene.h>

using namespace Rendering;

class ApplicationState {
    Scene m_scene;

    /*  For displaying FPS */
    clock_t m_currTime, m_prevTime;
    int m_frameCount;
    float m_secCount;        /*  Num of seconds from prevTime to currTime */
    float m_deltaTime;
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
    void UpdateTime() {
        ++m_frameCount;

        m_prevTime = m_currTime;
        m_currTime = clock();
        m_deltaTime = static_cast<float>(m_currTime - m_prevTime) / CLOCKS_PER_SEC;
        m_secCount += m_deltaTime;

        if (m_secCount > 1.f)
        {
            m_fps = m_frameCount / m_secCount;

            m_frameCount = 0;
            m_secCount = 0;
        }

    }
};
