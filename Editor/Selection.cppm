module;
#include "EditorExport.h"
export module Editor.Selection;
import Core;
import Editor.EditingContextId;
import EventBus;
import Thread;
import World;

export namespace Editor
{
    class EDITOR_API Selection : ThreadOwned, NoCopy, NoMove
    {
    public:
        Selection(World& world, EventBus& events, EditingContextId contextId);

        void add(Entity entity);
        void remove(Entity entity);
        void clear();
        void set(std::span<const Entity> entities);
        void setSingle(Entity entity);
        std::span<const Entity> get() const;
        bool contains(Entity entity) const;
        bool isEmpty() const;

    private:
        void sendSelectionEvent() const;
        EventBus& m_events;
        EditingContextId m_contextId;
        std::vector<Entity> m_entities;
        EventSubscription m_subscription;
    };
}
