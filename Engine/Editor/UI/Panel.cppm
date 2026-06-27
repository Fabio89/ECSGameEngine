export module UI.Panel;
export import World;
export import UI.ImGui;
import UI.IPanel;

export class Panel : public IPanel
{
public:
    explicit Panel(World& world) : m_world{&world} {}

    Panel(const Panel&) = delete;
    Panel(Panel&&) = delete;
    Panel& operator=(const Panel&) = delete;
    Panel& operator=(Panel&&) = delete;
    ~Panel() override = default;
    
    void draw() final { doDraw(*m_world); }

private:
    virtual void doDraw(World& world) = 0;
    World* m_world{};
};
