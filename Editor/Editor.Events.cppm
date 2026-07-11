export module Editor.Events;
import Core;
import Editor.EditingContextId;
import Editor.EntityEditingMode;

export namespace EditorEvents
{
    struct SelectionChanged
    {
        EditingContextId contextId;
        std::span<const Entity> selection;
    };

    struct EntityEditingModeChanged
    {
        EditingContextId contextId;
        EntityEditingMode mode;
    };
}
