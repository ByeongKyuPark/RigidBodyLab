#include <core/ApplicationState.h>

ApplicationState::ApplicationState()
    :m_scene{}, m_currTime{ clock() }, m_prevTime{},
    m_frameCount{}, m_secCount{}, m_deltaTime{}, m_fps{}
{}

float ApplicationState::GetDeltaTime() const {
    return m_deltaTime;
}
