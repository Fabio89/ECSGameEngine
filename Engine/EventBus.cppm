export module EventBus;
import Core;

using Callback = std::function<void(const void*)>;

export using SubscriptionId = Id<struct SubscriptionIdTag>;

struct Subscriber
{
    SubscriptionId id;
    Callback callback;
};

struct EventView
{
    TypeId type;
    const void* data{};
};

template<typename T>
struct FunctionTraits;

template<typename R, typename C, typename Arg>
struct FunctionTraits<R(C::*)(Arg) const>
{
    using Event = std::remove_cvref_t<Arg>;
};

export class EventBus
{
public:
    class Subscription;

    template<typename Func> [[nodiscard]]
    Subscription subscribe(Func&& callback);

    template<typename Event>
    void publish(const Event& event);

private:
    Subscription subscribe(TypeId event, Callback callback);

    void unsubscribe(TypeId event, SubscriptionId id);

    void publish(EventView event);

    std::unordered_map<TypeId, std::vector<Subscriber> > m_callbacks;
    SubscriptionId::ValueType m_nextSubscriptionId{};
};

class EventBus::Subscription
{
public:
    Subscription() = default;
    Subscription(const Subscription&) = delete;
    Subscription& operator=(const Subscription&) = delete;
    Subscription(Subscription&&) noexcept;
    Subscription& operator=(Subscription&&) noexcept;

    Subscription(EventBus* bus, TypeId event, SubscriptionId id)
        : m_bus(bus), m_event(event), m_id(id) {}

    ~Subscription();

private:
    void swap(Subscription& other) noexcept;

    void reset();

    EventBus* m_bus{};
    TypeId m_event{};
    SubscriptionId m_id{};
};

export class EventSubscription
{
public:
    void operator+=(EventBus::Subscription&&) noexcept;

private:
    std::vector<EventBus::Subscription> m_subs;
};

template<typename Func>
EventBus::Subscription EventBus::subscribe(Func&& callback)
{
    using Event = FunctionTraits<decltype(&std::remove_reference_t<Func>::operator())>::Event;

    Callback wrapper = [callback = std::forward<Func>(callback)](const void* event)
    {
        callback(*static_cast<const Event*>(event));
    };

    return subscribe(getTypeId<Event>(), std::move(wrapper));
}

template<typename Event>
void EventBus::publish(const Event& event)
{
    publish({getTypeId<Event>(), &event});
}
