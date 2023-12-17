#include <core/ApplicationState.h>

ApplicationState::ApplicationState()
    :m_scene{}, m_currTime{ clock() }, m_prevTime{},
    m_frameCount{}, m_secCount{}, m_deltaTime{}, m_fps{}
{}

float ApplicationState::GetDeltaTime() const {
    return m_deltaTime;
}

void ApplicationState::UpdateTime() {
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
