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
    EditingContextManager contextManager;
    std::unordered_map<EditingContextId, ControllerManager> controllerManagers;

    void addPanel(std::unique_ptr<Panel> panel);

    void init();

    void shutdown();

    bool update();
}

export namespace Editor
{
    EDITOR_API void run();

    template<typename T>
    EDITOR_API void request(T&& request) { requests.push(EditorRequest{std::forward<T>(request)}); }

    template<typename T>
    void addPanel(EditingContextId contextId) { addPanel(std::make_unique<T>(PanelCreateInfo{.contextId = contextId, .window = editorContext.window})); }

    std::span<Panel*> getPanels();

    template<typename T>
    T& addController(EditingContextId contextId) { return controllerManagers[contextId].addController<T>(contextManager.get(contextId)); }

    EditingContextManager& contexts() { return contextManager; }
}
