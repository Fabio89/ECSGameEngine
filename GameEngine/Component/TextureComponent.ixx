export module Engine.Component.Texture;
import Engine.Render.Core;
import Engine.Core;

export struct TextureComponent : Component<TextureComponent>
{
    std::string m_path;
    vk::Image m_image{nullptr};
    vk::DeviceMemory m_memory{nullptr};
};

class TextureSystem : public System
{
public:
    void addEntity(Entity entity, World& manager)
    {
        TextureComponent& texture = manager.editComponent<TextureComponent>(entity);
        manager.observeOnComponentAdded([this](Entity entity, ComponentTypeId componentType) { onComponentAdded(entity, componentType); });
    }

private:
    void onComponentAdded(Entity entity, ComponentTypeId componentType)
    {
        if (componentType == TextureComponent::typeId)
        {
        
        }
    }
};
