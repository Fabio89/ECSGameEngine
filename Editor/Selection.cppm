module;
#include "EditorExport.h"
export module Editor.Selection;
import Core;
import Editor.EditingContextId;
import Thread;

export namespace Editor
{
    class EDITOR_API Selection : ThreadOwned
    {
    public:
        Selection(EditingContextId contextId);

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
        EditingContextId m_contextId;
        std::vector<Entity> m_entities;
    };
}
