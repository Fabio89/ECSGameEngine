module Editor.SelectionGizmo;
import Editor;
import Engine.WorldManager;
import Editor.Events;
import Editor.Gizmos;
import World.Events;

SelectionGizmoManager::SelectionGizmoManager(EditorServices& services, EditingContext& context)
{
    WorldManager& worldManager = services.worlds;

    m_sub += services.events.subscribe([this, &worldManager, &context](const EditorEvents::SelectionChanged& event)
    {
        setSelectedEntities(worldManager.get(context.world), event.selection);
    });

    m_sub += worldManager.subscribe([this, &worldManager](const WorldEvents::SceneLoaded& event)
    {
        setSelectedEntities(worldManager.get(event.world), {});
        check(m_entitiesToBoundingBoxGizmos.empty(), "`SelectionGizmoManager::m_entitiesToBoundingBoxGizmos` should have been empty at this point!");
    });
}

void SelectionGizmoManager::setSelectedEntities(World& world, std::span<const Entity> entities)
{
    std::vector<Entity> toRemove;
    for (const Entity entity : m_entitiesToBoundingBoxGizmos | std::views::keys)
    {
        if (!std::ranges::contains(entities, entity))
            toRemove.push_back(entity);
    }

    for (const Entity entity : toRemove)
        destroyGizmo(world, entity);

    for (const Entity entity : entities)
    {
        if (check(!m_entitiesToBoundingBoxGizmos.contains(entity), "Tried to add selection gizmo to an entity more than once!"))
            m_entitiesToBoundingBoxGizmos.emplace(entity, SelectionGizmo{world, entity}).first;
    }
}

void SelectionGizmoManager::destroyGizmo(World& world, Entity attachedToEntity)
{
    if (auto it = m_entitiesToBoundingBoxGizmos.find(attachedToEntity); it != m_entitiesToBoundingBoxGizmos.end())
    {
        const Entity gizmo = it->second.getGizmoEntity();
        if (world.isValid(gizmo))
            world.removeEntity(gizmo);
        m_entitiesToBoundingBoxGizmos.erase(it);
    }
}

SelectionGizmo::SelectionGizmo(World& world, Entity attachTo)
    : m_world{world},
      m_gizmo{Gizmos::createBoundingBoxGizmo(world, attachTo)} {}

Entity SelectionGizmo::getGizmoEntity() const
{
    return m_gizmo;
}
