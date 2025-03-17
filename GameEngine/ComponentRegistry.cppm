export module Engine:ComponentRegistry;
import :Core;
import :Serialisation;
import :World;
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
    [[nodiscard]] virtual std::string getActualName() const = 0;
    [[nodiscard]] virtual std::string getDisplayName() const = 0;
    virtual void createInstance(World& world, Entity entity, const JsonObject& data) const = 0;
};

export template<ValidComponent T>
class ComponentType final : public ComponentTypeBase
{
public:
    explicit ComponentType(std::string displayName) : m_displayName{std::move(displayName)}
    {
        const std::string actualName{typeid(T).name()};

        const size_t endPos = actualName.find_last_not_of(' ');
        size_t startPos = actualName.find_last_of(' ', endPos);
        if (startPos == std::string::npos)
            startPos = 0;

        m_actualName = actualName.substr(startPos + 1, endPos - startPos);
        if (m_displayName.empty())
            m_displayName = m_actualName;

        std::cout << "Registered component `" << m_actualName << "`" << std::endl;
    }

    [[nodiscard]] ComponentTypeId getTypeId() const override { return T::typeId; }
    [[nodiscard]] std::string getActualName() const override { return m_actualName; }
    [[nodiscard]] std::string getDisplayName() const override { return m_displayName; }
    void createInstance(World& world, Entity entity, const JsonObject& data) const override { world.addComponent<T>(entity, deserialize<T>(data)); }

private:
    std::string m_actualName;
    std::string m_displayName;
};

namespace ComponentRegistry
{
    using ComponentCreateFunc = std::function<void(World&, Entity, const JsonObject&)>;

    std::vector<std::unique_ptr<const ComponentTypeBase>> componentTypes;
    std::map<ComponentTypeId, const ComponentTypeBase*> byId;
    std::map<std::string, const ComponentTypeBase*> byName;

    export template<ValidComponent T>
    void init(std::string displayName = {})
    {
        check(std::none_of(componentTypes.cbegin(), componentTypes.cend(), [](auto&& type) { return type->getTypeId() == T::typeId; }), "Tried to init components more than once!");
        const std::unique_ptr<const ComponentTypeBase>& type = componentTypes.emplace_back(std::make_unique<ComponentType<T>>(std::move(displayName)));
        byId[type->getTypeId()] = type.get();
        byName[type->getActualName()] = type.get();
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
