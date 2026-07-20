export module Editor.Panel;
export import Editor.EditingContextId;
export import Editor.PanelCreateInfo;
import std;

export class Panel
{
public:
    explicit Panel(const PanelCreateInfo& info)
        : m_window{info.window} {}

    Panel(const Panel&) = delete;

    Panel(Panel&&) = delete;

    Panel& operator=(const Panel&) = delete;

    Panel& operator=(Panel&&) = delete;

    virtual ~Panel() = default;

    void draw() { doDraw(); }

protected:
    WindowHandle getWindow() const { return m_window; }

private:
    virtual void doDraw() = 0;

    WindowHandle m_window;
};
