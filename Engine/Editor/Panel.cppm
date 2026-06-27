export module Editor.Panel;
import World;

export class Panel
{
public:
    explicit Panel(World& world) : m_world{&world} {}

    Panel(const Panel&) = delete;
    Panel(Panel&&) = delete;
    Panel& operator=(const Panel&) = delete;
    Panel& operator=(Panel&&) = delete;
    virtual ~Panel() = default;
    
    void draw() { doDraw(*m_world); }

private:
    virtual void doDraw(World& world) = 0;
    World* m_world{};
};
