module Editor.Selection;
import Editor.Events;
import Editor;
import Engine;
import Thread;
import World.Events;

Editor::Selection::Selection(World& world, EventBus& events, EditingContextId contextId)
    : m_events{events},
      m_contextId{contextId}
{
    m_subscription.clear();

    m_subscription += world.subscribe([this](const WorldEvents::WorldCleared&)
    {
        clear();
    });

    m_subscription += world.subscribe([this](const WorldEvents::EntityDestroyed& event)
    {
        remove(event.entity);
    });
}

void Editor::Selection::add(Entity entity)
{
    assertThread();
    if (!contains(entity))
    {
        m_entities.push_back(entity);
        sendSelectionEvent();
    }
}

void Editor::Selection::remove(Entity entity)
{
    assertThread();
    if (std::erase(m_entities, entity) != 0)
        sendSelectionEvent();
}

void Editor::Selection::clear()
{
    assertThread();
    if (!m_entities.empty())
    {
        m_entities.clear();
        sendSelectionEvent();
    }
}

void Editor::Selection::set(std::span<const Entity> entities)
{
    assertThread();

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

    sendSelectionEvent();
}

void Editor::Selection::setSingle(Entity entity)
{
    set(std::array{entity});
}

std::span<const Entity> Editor::Selection::get() const
{
    return m_entities;
}

bool Editor::Selection::contains(Entity entity) const
{
    return std::ranges::contains(m_entities, entity);
}

bool Editor::Selection::isEmpty() const
{
    return m_entities.empty();
}

void Editor::Selection::sendSelectionEvent() const
{
    m_events.publish(EditorEvents::SelectionChanged{.contextId = m_contextId, .selection = m_entities});
}
