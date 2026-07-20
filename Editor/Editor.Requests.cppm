export module Editor.Requests;
import Core;
import Editor.Controller;
import Editor.EditingContextId;
import Editor.EditingContext;
import Editor.Services;
import Engine.Viewport;
import Geometry;

export namespace Editor
{
    struct AddController
    {
        EditingContextId contextId;
        std::function<std::unique_ptr<EditorController>(EditorServices&, EditingContext&)> factory;
    };
}

export using EditorRequest = std::variant<
    Editor::AddController
>;
