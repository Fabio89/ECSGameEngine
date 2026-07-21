export module Editor.Controller;
import Core;
import Editor.EditingContext;
import Editor.PanelCreateInfo;
import Editor.Services;
import Editor.SnapshotFrame;
import ThreadSafeQueue;

export class EditorController : protected EditorServiceConsumer, NoCopy, NoMove
{
public:
    explicit EditorController(EditorServices& services, EditingContext& context)
        : EditorServiceConsumer{services},
          m_context{context}
    {}

    virtual ~EditorController() = default;

    virtual void update(float dt, SnapshotFrame& frame) {}

protected:
    EditingContext& context() { return m_context; }
    const EditingContext& context() const { return m_context; }

private:
    EditingContext& m_context;
};

export template<typename Controller>
class ControllerMailbox;

export template<typename TController, typename TSnapshot, typename TRequest>
class EditorControllerImpl : public EditorController
{
public:
    using SharedMailbox = std::shared_ptr<ControllerMailbox<TController>>;
    using Request = TRequest;
    using Snapshot = TSnapshot;

    EditorControllerImpl(EditorServices& services, EditingContext& context, SharedMailbox mailbox = nullptr)
        : EditorController{services, context},
          m_mailbox{std::move(mailbox)} {}

    static SharedMailbox createMailbox() { return std::make_shared<ControllerMailbox<TController>>(); }

    void update(float dt, SnapshotFrame& frame) override
    {
        EditorController::update(dt, frame);

        if (m_mailbox)
        {
            Request request;
            while (m_mailbox->m_queue.tryPop(request))
                std::visit([this]<typename R>(R&& r) { static_cast<TController*>(this)->execute(std::forward<R>(r)); }, std::move(request));
        }

        if constexpr (!std::same_as<Snapshot, NoController::Snapshot>)
            frame.set(buildSnapshot(context()));
    }

    void execute(const Request& request){}

private:
    using Type = TController;
    virtual Snapshot buildSnapshot(const EditingContext& context) { return {}; }

    SharedMailbox m_mailbox{};
};

template<typename Controller>
class ControllerMailbox
{
public:
    void send(Controller::Request request)
    {
        m_queue.push(std::move(request));
    }

private:
    friend class EditorControllerImpl<Controller, typename Controller::Snapshot, typename Controller::Request>;

    ThreadSafeQueue<typename Controller::Request> m_queue;
};

namespace Editor
{
    export class ControllerManager : EditorServiceConsumer
    {
    public:
        explicit ControllerManager(EditorServices& services) : EditorServiceConsumer{services} {}

        template<typename T>
        void addController(EditingContext& context, T::SharedMailbox mailbox);
        void addController(std::unique_ptr<EditorController> controller);

        void init();

        void update(float dt, SnapshotPublisher& snapshotPublisher);

    private:
        std::vector<std::unique_ptr<EditorController>> m_controllers;
    };
}

template<typename T>
void Editor::ControllerManager::addController(EditingContext& context, typename T::SharedMailbox mailbox)
{
    addController(std::make_unique<T>(services(), context, mailbox));
}

void Editor::ControllerManager::addController(std::unique_ptr<EditorController> controller)
{
    m_controllers.push_back(std::move(controller));
}

void Editor::ControllerManager::init() {}

void Editor::ControllerManager::update(float dt, SnapshotPublisher& snapshotPublisher)
{
    SnapshotFramePtr frame = snapshotPublisher.beginWrite();

    for (std::unique_ptr<EditorController>& controller : m_controllers)
    {
        controller->update(dt, *frame);
    }

    snapshotPublisher.publish(frame);
}
