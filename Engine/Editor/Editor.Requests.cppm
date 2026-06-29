export module Editor.Requests;
import Core;
import Editor.EditingContextId;

export namespace Editor::Requests
{
    struct ChangeSelection
    {
        EditingContextId contextId;
        std::vector<Entity> entities;
    };
}

export using EditorRequest = std::variant<
    Editor::Requests::ChangeSelection
>;
