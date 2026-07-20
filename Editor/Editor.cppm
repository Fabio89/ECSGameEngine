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
    void request(EditorRequest request);
    PanelCreateInfo generatePanelInfo(EditingContextId contextId);
    void addPanel(std::unique_ptr<Panel> panel);
    ControllerManager& ensureControllerManager(EditingContextId contextId);
}

export namespace Editor
{
    EDITOR_API void run();

    template<typename T>
    EDITOR_API void request(T&& payload) { request(EditorRequest{std::forward<T>(payload)}); }

    template<typename T>
    void addPanel(EditingContextId contextId) { addPanel(std::make_unique<T>(generatePanelInfo(contextId))); }

    std::span<Panel*> getPanels();

    EditingContextManager& contexts();

    void openProject(EditingContextId contextId, const std::filesystem::path& path);

    template<typename T, typename... Args>
    void addController(EditingContextId contextId, typename T::SharedMailbox mailbox, Args... args)
    {
        auto factory = [mailbox, args...](EditorServices& services, EditingContext& context)
        {
            return std::make_unique<T>(services, context, std::move(mailbox), args...);
        };

        request(AddController{.contextId = contextId, .factory = std::move(factory)});
    }
}
