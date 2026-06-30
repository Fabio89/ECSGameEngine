export module Editor.Events;
import Core;
import Editor.EditingContextId;

export namespace Editor
{
    struct SelectionChangedEvent
    {
        EditingContextId contextId;
        std::span<const Entity> selection;
    };
}
