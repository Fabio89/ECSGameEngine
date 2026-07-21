module Engine.FrameTimer;

FrameTimer::FrameTimer()
    : m_lastFrame{Clock::now()}
{
}

float FrameTimer::tick()
{
    return tick(0.f);
}

float FrameTimer::tick(float targetHz)
{
    auto now = Clock::now();
    auto elapsed = now - m_lastFrame;

    if (targetHz > 0.f)
    {
        if (const auto target = std::chrono::duration<float>(1.f / targetHz); elapsed < target)
        {
            std::this_thread::sleep_for(target - elapsed);

            now = Clock::now();
            elapsed = now - m_lastFrame;
        }
    }

    m_lastFrame = now;

    m_deltaTime = std::chrono::duration<float>(elapsed).count();
    m_deltaTime = std::min(m_deltaTime, 0.1f);

    m_elapsedSeconds += m_deltaTime;
    ++m_frameNumber;

    return m_deltaTime;
}
