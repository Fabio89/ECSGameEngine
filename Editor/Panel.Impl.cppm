export module Editor.Panel.Impl;
export import Editor.EditingContext;
export import Editor.Panel;
export import Editor.PanelCreateInfo;
export import World;
import Editor;
import std;

export class PanelImpl : public Panel
{
public:
    explicit PanelImpl(const PanelCreateInfo& info);

protected:
    EditingContext& context();
    const EditingContext& context() const;

private:
    std::reference_wrapper<EditingContext> m_context;
};

PanelImpl::PanelImpl(const PanelCreateInfo& info)
    : Panel{info},
      m_context{Editor::contexts().get(info.contextId)} {}

EditingContext& PanelImpl::context()
{
    return m_context;
}

const EditingContext& PanelImpl::context() const
{
    return m_context;
}
