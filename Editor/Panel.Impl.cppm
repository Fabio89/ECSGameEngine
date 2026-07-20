export module Editor.Panel.Impl;
export import Editor.EditingContext;
export import Editor.Panel;
export import Editor.PanelCreateInfo;
export import Editor.SnapshotFrame;
import Editor;
import std;

export template<ControllerTraits Controller = NoController>
class PanelImpl : public Panel
{
public:
    explicit PanelImpl(const PanelCreateInfo& info)
        : Panel{info},
          m_context{Editor::contexts().get(info.contextId)},
          m_mailbox{Controller::createMailbox()} {}

protected:
    using Snapshot = Controller::Snapshot;
    EditingContext& context() { return m_context; }
    const EditingContext& context() const { return m_context; }

    template<typename... Args>
    void createController(Args&&... args) requires (!std::same_as<Controller, NoController>)
    {
        Editor::addController<Controller>(context().id, m_mailbox, std::forward<Args>(args)...);
    }

    void request(Controller::Request request) requires (!std::same_as<Controller, NoController>)
    {
        if (m_mailbox)
            m_mailbox->send(std::move(request));
    }

    const Snapshot* getSnapshot() const requires (!std::same_as<Controller, NoController>)
    {
        const Editor::SnapshotFrame& frame = context().snapshotPublisher.frame();
        return frame.contains<Snapshot>() ? &frame.get<Snapshot>() : nullptr;
    }

private:
    EditingContext& m_context;

    [[no_unique_address]]
    Controller::SharedMailbox m_mailbox;
};

