export module EditorUI;
import Core;
import UI.IPanel;

export class EditorUI
{
public:
    static constexpr bool enabled = true;

    void addPanel(std::unique_ptr<IPanel> panel);

    void draw();

    static bool isMouseAvailable();

    static bool isKeyboardAvailable();

private:
    std::vector<std::unique_ptr<IPanel>> m_panels;
};
