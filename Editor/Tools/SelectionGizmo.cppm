export module Editor.SelectionGizmo;
import Core;
import Editor.EditingContext;
import EditorUIContext;
import EventBus;
import World;

class SelectionGizmo
{
public:
    SelectionGizmo(World& world, Entity attachTo);
    Entity getGizmoEntity() const;

private:
    std::reference_wrapper<World> m_world;
    Entity m_gizmo;
};

export class SelectionGizmoManager
{
public:
    SelectionGizmoManager(EditingContext& context);

private:
    void setSelectedEntities(World& world, std::span<const Entity> entities);

    void destroyGizmo(World& world, Entity attachedToEntity);

    std::unordered_map<Entity, SelectionGizmo> m_entitiesToBoundingBoxGizmos;
    EventSubscription m_sub;
};
