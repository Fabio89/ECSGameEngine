export module Editor.Requests;
import Core;
import Editor.EditingContextId;
import Properties;

export namespace Editor
{
    struct ChangeSelection
    {
        EditingContextId contextId;
        std::vector<Entity> entities;
    };

    struct OpenProject
    {
        EditingContextId contextId;
        std::filesystem::path path;
    };

    struct OpenScene
    {
        EditingContextId contextId;
        std::filesystem::path path;
    };

    struct SetProperty
    {
        EditingContextId contextId;
        Entity entity;
        TypeId componentType;
        const PropertyDescriptorBase* property;
        PropertyValue value;
    };
}

export using EditorRequest = std::variant<
    Editor::ChangeSelection,
    Editor::OpenProject,
    Editor::OpenScene,
    Editor::SetProperty
>;
