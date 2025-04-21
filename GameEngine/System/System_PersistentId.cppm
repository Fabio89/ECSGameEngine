export module System.PersistentId;
import Component.PersistentId;
import System;

export class System_PersistentId final : public System
{
    void onComponentAdded(World& world, Entity entity, ComponentTypeId componentType) override
    {
        if (componentType == getComponentType<PersistentIdComponent>())
        {
            PersistentIdUtils::registerEntity(entity, world.readComponent<PersistentIdComponent>(entity).id);
        }
    }

    void onUpdate(World& world, [[maybe_unused]] Player& player, [[maybe_unused]] float deltaTime) override
    {
    }
};
