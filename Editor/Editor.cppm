module;

#include "EditorExport.h"

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
    EDITOR_API void init(EditorUIContext context);

    EDITOR_API void shutdown();

    EDITOR_API void update();

    template<typename T>
    EDITOR_API void request(T&& request) { requests.push(EditorRequest{std::forward<T>(request)}); }

    template<typename T>
    EDITOR_API void addPanel(EditingContextId contextId) { addPanel(std::make_unique<T>(PanelCreateInfo{.contextId = contextId, .window = editorContext.window})); }

    std::span<Panel*> getPanels();

    template<typename T>
    EDITOR_API T& addController(EditingContextId contextId) { return controllerManager.addController<T>(contextManager.get(contextId)); }

    EDITOR_API EditingContextManager& contexts() { return contextManager; }
}
