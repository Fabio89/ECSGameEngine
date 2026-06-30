export module Editor.Controller;
import Editor.EditingContext;
import std;

export class EditorController
{
public:
    explicit EditorController(EditingContext& contextId) : m_context{contextId} {}
    virtual ~EditorController() = default;

    virtual void update(float dt) {}

private:
    std::reference_wrapper<EditingContext> m_context;
};

namespace Editor
{
    export class ControllerManager
    {
    public:
        template<typename T>
        T& addController(EditingContext& context);

        void init();
        void update(float dt);

    private:
        std::vector<std::unique_ptr<EditorController>> m_controllers;
    };
}

void Editor::ControllerManager::init()
{
}

void Editor::ControllerManager::update(float dt)
{
    for (std::unique_ptr<EditorController>& controller : m_controllers)
    {
        controller->update(dt);
    }
}

template<typename T>
T& Editor::ControllerManager::addController(EditingContext& context)
{
    std::unique_ptr<T> controller = std::make_unique<T>(context);
    T* ptr = controller.get();
    m_controllers.push_back(std::move(controller));
    return *ptr;
}
