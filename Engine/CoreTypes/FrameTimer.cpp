module Engine.FrameTimer;

FrameTimer::FrameTimer()
    : m_lastFrame{Clock::now()}
{
}

float FrameTimer::tick()
{
    const Clock::time_point now{Clock::now()};

    m_deltaTime = std::chrono::duration<float>(now - m_lastFrame).count();
    m_lastFrame = now;
    m_deltaTime = std::min(m_deltaTime, 0.1f);
    m_elapsedSeconds += m_deltaTime;
    ++m_frameNumber;

    return m_deltaTime;
}

void FrameTimer::waitForTarget(float hz) const
{
    if (const float target = 1.0f / hz; m_deltaTime < target)
    {
        std::this_thread::sleep_for(std::chrono::duration<float>(target - m_deltaTime));
    }
}
