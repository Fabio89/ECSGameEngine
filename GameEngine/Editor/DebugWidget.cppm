export module Engine:DebugWidget;

export class World;

export class DebugWidget
{
public:
    DebugWidget(const DebugWidget&) = delete;
    DebugWidget(DebugWidget&&) = delete;
    DebugWidget& operator=(const DebugWidget&) = delete;
    DebugWidget& operator=(DebugWidget&&) = delete;
    
    virtual ~DebugWidget() = default;
    explicit DebugWidget(World& world) : m_world{&world} {}
    void draw() { draw(*m_world); }

private:
    virtual void draw(World& world) = 0;
    World* m_world{};
};
