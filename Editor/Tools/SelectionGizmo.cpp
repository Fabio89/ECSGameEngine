module Editor.SelectionGizmo;
import Editor;
import Engine.WorldManager;
import Editor.Events;
import Editor.Gizmos;
import World.Events;

SelectionGizmoManager::SelectionGizmoManager(EditorServices& services, EditingContext& context)
    : m_editorWorld{services.worlds.get(context.editorWorld)}
{
    WorldManager& worldManager = services.worlds;

    m_sub += services.events.subscribe([this, &worldManager, &context](const EditorEvents::SelectionChanged& event)
    {
        setSelectedEntities(worldManager.get(context.world), event.selection);
    });
}

void SelectionGizmoManager::setSelectedEntities(const World& world, std::span<const Entity> entities)
{
    std::vector<Entity> toRemove;
    for (const Entity entity : m_entitiesToBoundingBoxGizmos | std::views::keys)
    {
        if (!std::ranges::contains(entities, entity))
            toRemove.push_back(entity);
    }

    for (const Entity entity : toRemove)
        destroyGizmo(m_editorWorld, entity);

    for (const Entity entity : entities)
    {
        if (check(!m_entitiesToBoundingBoxGizmos.contains(entity), "Tried to add selection gizmo to an entity more than once!"))
            m_entitiesToBoundingBoxGizmos.emplace(entity, SelectionGizmo{m_editorWorld, world, entity}).first;
    }
}

void SelectionGizmoManager::destroyGizmo(World& editorWorld, Entity sourceEntity)
{
    if (const auto node = m_entitiesToBoundingBoxGizmos.extract(sourceEntity); !node.empty())
    {
        if (const Entity gizmo = node.mapped().getGizmoEntity(); editorWorld.isValid(gizmo))
            editorWorld.removeEntity(gizmo);
    }
}

SelectionGizmo::SelectionGizmo(World& editorWorld, const World& sourceEntityWorld, Entity sourceEntity)
    : m_gizmo{Gizmos::createBoundingBoxGizmo(editorWorld, sourceEntityWorld, sourceEntity)} {}

Entity SelectionGizmo::getGizmoEntity() const
{
    return m_gizmo;
}
