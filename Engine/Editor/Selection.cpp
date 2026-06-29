module Editor.Selection;
import Editor.Events;
import Engine;
import Thread;

Editor::Selection::Selection(EditingContextId contextId) : m_contextId{contextId}
{
}

void Editor::Selection::add(Entity entity)
{
    if (!contains(entity))
        m_entities.push_back(entity);
}

void Editor::Selection::remove(Entity entity)
{
    std::erase(m_entities, entity);
}

void Editor::Selection::clear()
{
    m_entities.clear();
}

bool Editor::Selection::contains(Entity entity) const
{
    return std::ranges::contains(m_entities, entity);
}

bool Editor::Selection::isEmpty() const
{
    return m_entities.empty();
}

void Editor::Selection::set(std::span<const Entity> entities)
{
    Thread::assertGameThread();

    std::vector<Entity> newSelection;
    newSelection.reserve(entities.size());

    for (Entity e : entities)
    {
        if (e.isValid())
            newSelection.push_back(e);
    }

    if (m_entities == newSelection)
        return;

    m_entities = std::move(newSelection);

    Engine::events().publish(SelectionChangedEvent{.contextId = m_contextId, .selection = m_entities});
}

void Editor::Selection::setSingle(Entity entity)
{
    set(std::array{entity});
}

std::span<const Entity> Editor::Selection::get() const
{
    return m_entities;
}