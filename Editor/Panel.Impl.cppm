export module Editor.Panel.Impl;
export import Editor.EditingContext;
export import Editor.Panel;
export import Editor.PanelCreateInfo;
import Editor;
import std;

export class PanelImpl : public Panel
{
public:
    explicit PanelImpl(const PanelCreateInfo& info)
        : Panel{info},
          m_context{Editor::contexts().get(info.contextId)} {}

protected:
    EditingContext& context() { return m_context; }
    const EditingContext& context() const { return m_context; }

private:
    EditingContext& m_context;
};

