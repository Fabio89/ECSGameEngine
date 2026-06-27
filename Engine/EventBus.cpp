module EventBus;

EventBus::Subscription::Subscription(Subscription&& other) noexcept
{
    swap(other);
}

EventBus::Subscription& EventBus::Subscription::operator=(Subscription&& other) noexcept
{
    if (this != &other)
    {
        reset();
        swap(other);
    }
    return *this;
}

EventBus::Subscription::~Subscription()
{
    reset();
}

void EventBus::Subscription::swap(Subscription& other) noexcept
{
    std::swap(m_bus, other.m_bus);
    std::swap(m_event, other.m_event);
    std::swap(m_id, other.m_id);
}

void EventBus::Subscription::reset()
{
    if (m_bus)
    {
        m_bus->unsubscribe(m_event, m_id);
        m_bus = nullptr;
    }
}

void EventSubscription::operator+=(EventBus::Subscription&& subscription) noexcept
{
    m_subs.push_back(std::move(subscription));
}

EventBus::Subscription EventBus::subscribe(TypeId event, Callback callback)
{
    const SubscriptionId id{m_nextSubscriptionId++};
    m_callbacks[event].push_back({.id = id, .callback = std::move(callback)});

    return Subscription{this, event, id};
}

void EventBus::unsubscribe(TypeId event, SubscriptionId id)
{
    auto& subscribers = m_callbacks[event];

    std::erase_if(subscribers,
                  [&](const Subscriber& s)
                  {
                      return s.id == id;
                  });
}

void EventBus::publish(EventView event)
{
    if (auto it = m_callbacks.find(event.type); it != m_callbacks.end())
    {
        for (const Subscriber& subscriber : it->second)
        {
            subscriber.callback(event.data);
        }
    }
}
