module Editor.EditingContext;
import World;

EditingContext::EditingContext(EditingContextId id, World& world, WorldHandle editorWorld, EventBus& events)
    : id{id},
      world{world.getHandle()},
      editorWorld{editorWorld},
      selection{world, events, id} {}

EditingContextManager::EditingContextManager(WorldManager& worlds, EventBus& editorEvents)
    : m_worlds{worlds},
      m_editorEvents{editorEvents} {}

EditingContextId EditingContextManager::add(EditingContextCreateInfo info)
{
    const EditingContextId id{m_lastId++};
    World& world = m_worlds.get(info.world);
    m_contexts[id.value] = std::make_unique<EditingContext>(id, world, info.editorWorld, m_editorEvents);
    return id;
}

EditingContext& EditingContextManager::get(EditingContextId id) { return const_cast<EditingContext&>(std::as_const(*this).get(id)); }

const EditingContext& EditingContextManager::get(EditingContextId id) const
{
    if (auto it = m_contexts.find(id.value); it != m_contexts.end())
        return *it->second;

    report(std::format("Requested invalid EditingContextId: {}", id.value));
    static World invalidWorld;
    static EditingContext invalidContext{{}, invalidWorld, {}, m_editorEvents};
    return invalidContext;
}

void EditingContextManager::remove(EditingContextId contextId)
{
    m_contexts.erase(contextId.value);
}