export module Editor.Events;
import Core;

export namespace Editor
{
    struct SelectionChangedEvent
    {
        std::span<const Entity> selection;
    };
}