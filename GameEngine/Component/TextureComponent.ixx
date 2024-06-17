export module Engine.Component.Texture;
import Engine.Render.Core;
import Engine.Core;

export struct TextureComponent : Component<TextureComponent>
{
    vk::Image m_image;
    vk::DeviceMemory m_memory;
};

class TextureSystem : public System
{
public:
    void addEntity(Entity entity, World& manager)
    {
        TextureComponent& texture = manager.editComponent<TextureComponent>(entity);
        manager.observeOnComponentAdded([this](Entity entity, std::type_index componentType) { onComponentAdded(entity, componentType); });
    }

private:
    void onComponentAdded(Entity entity, std::type_index componentType)
    {
        if (componentType == TextureComponent::TypeID)
        {
        
        }
    }
};
