module Editor.EditingContext;

EditingContextId EditingContextManager::add(World& world)
{
    const EditingContextId id{m_lastId++};
    m_contexts[id.value] = std::make_unique<EditingContext>(EditingContext{
        .id = id,
        .world = world,
        .selection = Editor::Selection{id},
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
    static World invalidWorld{WorldCreateInfo{}};
    static EditingContext invalidContext{.world = invalidWorld, .selection = Editor::Selection{EditingContextId{}}};
    return invalidContext;
}

void EditingContextManager::remove(EditingContextId contextId)
{
    m_contexts.erase(contextId.value);
}