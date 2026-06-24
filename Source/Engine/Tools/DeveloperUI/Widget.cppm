export module DebugUI.DebugWidget;
import DevUI.IDebugWidget;
import World;

export class Widget : public IWidget
{
public:
    explicit Widget(World& world) : m_world{&world} {}

    Widget(const Widget&) = delete;
    Widget(Widget&&) = delete;
    Widget& operator=(const Widget&) = delete;
    Widget& operator=(Widget&&) = delete;
    ~Widget() override = default;
    
    void draw() final { doDraw(*m_world); }

private:
    virtual void doDraw(World& world) = 0;
    World* m_world{};
};
