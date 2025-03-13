export module Engine:DebugWidget;
import :IDebugWidget;
import :World;

export class DebugWidget : public IDebugWidget
{
public:
    explicit DebugWidget(World& world) : m_world{&world} {}

    DebugWidget(const DebugWidget&) = delete;
    DebugWidget(DebugWidget&&) = delete;
    DebugWidget& operator=(const DebugWidget&) = delete;
    DebugWidget& operator=(DebugWidget&&) = delete;
    ~DebugWidget() override = default;
    
    void draw() final { doDraw(*m_world); }

private:
    virtual void doDraw(World& world) = 0;
    World* m_world{};
};
