module Editor.EditingContext;

EditingContextManager::EditingContextManager(EditorServices& services) : m_services{services}
{
}

EditingContextId EditingContextManager::add(WorldHandle world)
{
    const EditingContextId id{m_lastId++};
    m_contexts[id.value] = std::make_unique<EditingContext>(EditingContext{
        .id = id,
        .world = world,
        .selection = Editor::Selection{m_services.events, id},
        .camera = {}
    });
    return id;
}

EditingContext& EditingContextManager::get(EditingContextId id) { return const_cast<EditingContext&>(std::as_const(*this).get(id)); }

const EditingContext& EditingContextManager::get(EditingContextId id) const
{
    if (auto it = m_contexts.find(id.value); it != m_contexts.end())
        return *it->second;

    report(std::format("Requested invalid EditingContextId: {}", id.value));
    static EditingContext invalidContext{.world = {}, .selection = Editor::Selection{m_services.events, EditingContextId{}}};
    return invalidContext;
}

void EditingContextManager::remove(EditingContextId contextId)
{
    m_contexts.erase(contextId.value);
}