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
import Editor.Services;
import ThreadSafeQueue;
import Window;
import World;

namespace Editor
{
    EditorUIContext editorContext{};
    ThreadSafeQueue<EditorRequest> requests;

    void addPanel(std::unique_ptr<Panel> panel);
    ControllerManager& ensureControllerManager(EditingContextId contextId);
}

export namespace Editor
{
    EDITOR_API void run();

    template<typename T>
    EDITOR_API void request(T&& request) { requests.push(EditorRequest{std::forward<T>(request)}); }

    template<typename T>
    void addPanel(EditingContextId contextId) { addPanel(std::make_unique<T>(PanelCreateInfo{.contextId = contextId, .window = editorContext.window})); }

    std::span<Panel*> getPanels();

    EditingContextManager& contexts();

    template<typename T>
    void addController(EditingContextId contextId)
    {
        auto factory = [](EditorServices& services, EditingContext& context) { return std::make_unique<T>(services, context); };
        request(AddController{.contextId = contextId, .factory = std::move(factory)});
    }
}
