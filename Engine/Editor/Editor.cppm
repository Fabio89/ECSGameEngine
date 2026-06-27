module;

#include "EngineExport.h"

export module Editor;
import Core;
import UI.IPanel;
import Window;
import World;

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

    ENGINE_API void init(World& world);

    ENGINE_API void createGizmos(World& world);

    ENGINE_API void update(World& world, WindowHandle window, float deltaTime);

    ENGINE_API void drawEditorUI();

    ENGINE_API void setSingleSelection(World& world, Entity entity);

    ENGINE_API void setSelection(World& world, std::span<const Entity> entities);

    ENGINE_API Selection& selection();

    ENGINE_API std::span<const Entity> getSelection();

    ENGINE_API void addPanel(std::unique_ptr<IPanel> panel);

    template<typename T>
    ENGINE_API void addPanel(World& world) { addPanel(std::make_unique<T>(world)); }
}
