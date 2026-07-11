export module Editor.Requests;
import Core;
import Editor.Controller;
import Editor.EditingContextId;
import Editor.EntityEditingMode;
import Editor.EditingContext;
import Editor.Services;
import Geometry;
import Properties;
import Window;

export namespace Editor
{
    struct ChangeSelection
    {
        EditingContextId contextId;
        std::vector<Entity> entities;
    };

    struct SelectEntityUnderCursor
    {
        EditingContextId contextId;
        WindowHandle window;
        Rect viewportArea;
    };

    struct SetEntityEditingMode
    {
        EditingContextId contextId;
        EntityEditingMode mode{EntityEditingMode::None};
    };

    struct AddController
    {
        EditingContextId contextId;
        std::function<std::unique_ptr<EditorController>(EditorServices&, EditingContext&)> factory;
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

    struct SetCameraMouseLookEnabled
    {
        WindowHandle window;
        bool enabled{};
    };
}

export using EditorRequest = std::variant<
    Editor::ChangeSelection,
    Editor::SelectEntityUnderCursor,
    Editor::SetEntityEditingMode,
    Editor::AddController,
    Editor::OpenProject,
    Editor::OpenScene,
    Editor::SetProperty,
    Editor::SetCameraMouseLookEnabled
>;
