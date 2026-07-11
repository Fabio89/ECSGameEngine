export module Editor.Controller;
import Editor.EditingContext;
import Editor.Services;
import Editor.SnapshotFrame;
import std;

export class EditorController
{
public:
    explicit EditorController(EditorServices& services, EditingContext& contextId) : m_services{services}, m_context{contextId} {}
    virtual ~EditorController() = default;

    virtual void update(float dt, Editor::SnapshotFrame& frame) {}

protected:
    EditorServices& services() { return m_services; }
    const EditorServices& services() const { return m_services; }

    EditingContext& context() { return m_context; }
    const EditingContext& context() const { return m_context; }

private:
    EditorServices& m_services;
    EditingContext& m_context;
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
        explicit ControllerManager(EditorServices& services) : m_services{services} {}

        template<typename T>
        void addController(EditingContext& context);
        void addController(std::unique_ptr<EditorController> controller);

        void init();
        void update(float dt, SnapshotPublisher& snapshotPublisher);

    private:
        EditorServices& m_services;
        std::vector<std::unique_ptr<EditorController>> m_controllers;
    };
}

template<typename T>
void Editor::ControllerManager::addController(EditingContext& context)
{
    addController(std::make_unique<T>(m_services, context));
}

void Editor::ControllerManager::addController(std::unique_ptr<EditorController> controller)
{
    m_controllers.push_back(std::move(controller));
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
