export module Editor.Controller;
import Editor.EditingContext;
import Editor.SnapshotFrame;
import std;

export class EditorController
{
public:
    explicit EditorController(EditingContext& contextId) : m_context{contextId} {}
    virtual ~EditorController() = default;

    virtual void update(float dt, Editor::SnapshotFrame& frame) {}

protected:
    EditingContext& context() { return m_context; }
    [[nodiscard]] const EditingContext& context() const { return m_context; }

private:
    std::reference_wrapper<EditingContext> m_context;
};

export template<typename Snapshot>
class EditorControllerImpl : public EditorController
{
public:
    using EditorController::EditorController;

    void update(float dt, Editor::SnapshotFrame& frame) override
    {
        EditorController::update(dt, frame);
        frame.set(buildSnapshot(context()));
    }

private:
    virtual Snapshot buildSnapshot(const EditingContext& context) = 0;
};

namespace Editor
{
    export class ControllerManager
    {
    public:
        template<typename T>
        T& addController(EditingContext& context);

        void init();
        void update(float dt, SnapshotPublisher& snapshotPublisher);

    private:
        std::vector<std::unique_ptr<EditorController>> m_controllers;
    };
}

void Editor::ControllerManager::init()
{
}

void Editor::ControllerManager::update(float dt, SnapshotPublisher& snapshotPublisher)
{
    SnapshotFrame& frame = snapshotPublisher.beginWrite();

    for (std::unique_ptr<EditorController>& controller : m_controllers)
    {
        controller->update(dt, frame);
    }

    snapshotPublisher.publish();
}

template<typename T>
T& Editor::ControllerManager::addController(EditingContext& context)
{
    std::unique_ptr<T> controller = std::make_unique<T>(context);
    T* ptr = controller.get();
    m_controllers.push_back(std::move(controller));
    return *ptr;
}
