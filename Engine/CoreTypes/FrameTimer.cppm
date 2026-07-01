export module Engine.FrameTimer;
import Core;
import std;

export class FrameTimer
{
public:
    FrameTimer();

    float tick();

    void waitForTarget(float hz) const;

    [[nodiscard]] float deltaTime() const { return m_deltaTime; }
    [[nodiscard]] double elapsedSeconds() const { return m_elapsedSeconds; }
    [[nodiscard]] Int64 frameNumber() const { return m_frameNumber; }

private:
    using Clock = std::chrono::steady_clock;

    Clock::time_point m_lastFrame;

    float m_deltaTime{};
    double m_elapsedSeconds{};
    Int64 m_frameNumber{};
};