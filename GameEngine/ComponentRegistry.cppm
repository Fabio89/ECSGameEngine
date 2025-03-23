export module ComponentRegistry;
import World;
import Serialization;
import std;

export class ComponentTypeBase
{
public:
    ComponentTypeBase() = default;
    virtual ~ComponentTypeBase() = default;
    ComponentTypeBase(const ComponentTypeBase&) = delete;
    ComponentTypeBase(ComponentTypeBase&&) = delete;
    ComponentTypeBase& operator=(const ComponentTypeBase&) = delete;
    ComponentTypeBase& operator=(ComponentTypeBase&&) = delete;

    [[nodiscard]] virtual ComponentTypeId getTypeId() const = 0;
    [[nodiscard]] virtual std::string_view getName() const = 0;
    virtual void createInstance(World& world, Entity entity, const JsonObject& data) const = 0; // Could be refactored out of this class
    [[nodiscard]] virtual JsonObject serialize(const ComponentBase& component, Json::MemoryPoolAllocator<>& allocator) const = 0;
    virtual void deserialize(World& world, Entity entity, const JsonObject& json) const = 0;
};

export template<ValidComponent T>
class ComponentType final : public ComponentTypeBase
{
public:
    [[nodiscard]] ComponentTypeId getTypeId() const override { return T::typeId; }
    [[nodiscard]] std::string_view getName() const override { return getComponentName<T>(); }
    void createInstance(World& world, Entity entity, const JsonObject& json) const override { world.addComponent<T>(entity, ::deserialize<T>(json)); }
    [[nodiscard]] JsonObject serialize(const ComponentBase& component, Json::MemoryPoolAllocator<>& allocator) const override { return ::serialize<T>(static_cast<const T&>(component), allocator); }
    void deserialize(World& world, Entity entity, const JsonObject& json) const override { world.editComponent<T>(entity) = ::deserialize<T>(json); }
};

namespace ComponentRegistry
{
    using ComponentCreateFunc = std::function<void(World&, Entity, const JsonObject&)>;

    std::vector<std::unique_ptr<const ComponentTypeBase>> componentTypes;
    std::map<ComponentTypeId, const ComponentTypeBase*> byId;
    std::map<std::string, const ComponentTypeBase*> byName;

    export template<ValidComponent T>
    void init()
    {
        check(std::none_of(componentTypes.cbegin(), componentTypes.cend(), [](auto&& type) { return type->getTypeId() == T::typeId; }), "Tried to init components more than once!");
        const std::unique_ptr<const ComponentTypeBase>& type = componentTypes.emplace_back(std::make_unique<ComponentType<T>>());
        byId[type->getTypeId()] = type.get();
        byName[getComponentName<T>()] = type.get();
        log(std::format("Registered component: {} (id={})", getComponentName<T>(), type->getTypeId()));
    }

    export const ComponentTypeBase* get(ComponentTypeId typeId)
    {
        auto it = byId.find(typeId);
        return it != byId.end() ? it->second : nullptr;
    }

    export const ComponentTypeBase* get(const std::string& name)
    {
        auto it = byName.find(name);
        return it != byName.end() ? it->second : nullptr;
    }
}
