export module Editor.SelectionGizmo;
import Core;
import Editor.EditingContext;
import Editor.Services;
import EditorUIContext;
import EventBus;
import World;

class SelectionGizmo
{
public:
    SelectionGizmo(World& editorWorld, const World& sourceEntityWorld, Entity sourceEntity);
    Entity getGizmoEntity() const;

private:
    Entity m_gizmo;
};

export class SelectionGizmoManager : NoCopy, NoMove
{
public:
    SelectionGizmoManager(EditorServices& services, EditingContext& context);

private:
    void setSelectedEntities(const World& world, std::span<const Entity> entities);

    void destroyGizmo(World& editorWorld, Entity sourceEntity);

    World& m_editorWorld;
    std::unordered_map<Entity, SelectionGizmo> m_entitiesToBoundingBoxGizmos;
    EventSubscription m_sub;
};
