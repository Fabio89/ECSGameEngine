module;

#include "EngineExport.h"

export module Editor;
export import EditorContext;
import Core;
import Editor.Panel;
import Window;
import World;

namespace Editor
{
    EditorContext editorContext{};
}

export namespace Editor
{
    class ENGINE_API Selection
    {
    public:
        void add(Entity entity);
        void remove(Entity entity);
        void clear();
        void set(std::span<const Entity> entities);
        void setSingle(Entity entity);
        std::span<const Entity> get() const;
        bool contains(Entity entity) const;
        bool isEmpty() const;

    private:
        std::vector<Entity> m_entities;
    };

    ENGINE_API void init(EditorContext context);

    ENGINE_API void shutdown();

    ENGINE_API void createGizmos();

    ENGINE_API void update(float deltaTime);

    ENGINE_API void setSingleSelection(Entity entity);

    ENGINE_API void setSelection(std::span<const Entity> entities);

    ENGINE_API Selection& selection();

    ENGINE_API std::span<const Entity> getSelection();

    ENGINE_API void addPanel(std::unique_ptr<Panel> panel);

    template<typename T>
    ENGINE_API void addPanel() { addPanel(std::make_unique<T>(*editorContext.world)); }
}
