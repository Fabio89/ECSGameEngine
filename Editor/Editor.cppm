module;

#include "EngineExport.h"

export module Editor;
export import EditorUIContext;
import Core;
import Editor.Controller;
import Editor.EditingContext;
import Editor.Panel;
import Editor.Requests;
import Editor.Selection;
import CoreTypes;
import Window;
import World;

namespace Editor
{
    EditorUIContext editorContext{};
    ThreadSafeQueue<EditorRequest> requests;
    ControllerManager controllerManager;
    EditingContextManager contextManager;

    void addPanel(std::unique_ptr<Panel> panel);
}

export namespace Editor
{
    ENGINE_API void init(EditorUIContext context);

    ENGINE_API void shutdown();

    ENGINE_API void update(float deltaTime);

    template<typename T>
    ENGINE_API void request(T&& request) { requests.push(EditorRequest{std::forward<T>(request)}); }

    template<typename T>
    ENGINE_API void addPanel(EditingContextId contextId) { addPanel(std::make_unique<T>(PanelCreateInfo{.contextId = contextId, .window = editorContext.window})); }

    template<typename T>
    ENGINE_API T& addController(EditingContextId contextId) { return controllerManager.addController<T>(contextManager.get(contextId)); }

    ENGINE_API EditingContextManager& contexts() { return contextManager; }
}
