export module Render.RenderWorld;
import Render.RenderObject;
import Render.VulkanResource;
import WorldHandle;

export struct RenderWorldCreateInfo
{
    WorldHandle world;
    vk::DescriptorPool descriptorPool{};
    vk::DescriptorSetLayout descriptorSetLayout{};
};

export class RenderWorld : VulkanResource
{
public:
    RenderWorld(VulkanContext& context, const RenderWorldCreateInfo& info);
    ~RenderWorld();

    void drawFrame(const RenderPassContext& renderContext);
    [[nodiscard]] RenderObjectManager& objects();
    [[nodiscard]] const RenderObjectManager& objects() const;

private:
    WorldHandle m_world;
    RenderObjectManager m_objects;
};

export struct RenderWorldManagerContext
{
    vk::DescriptorPool& descriptorPool;
    vk::DescriptorSetLayout& descriptorSetLayout;
};

export class RenderWorldManager : VulkanResource
{
public:
    explicit RenderWorldManager(VulkanContext& vulkanContext, const RenderWorldManagerContext& worldManagerContext);

    void registerWorld(WorldHandle world);
    void unregisterWorld(WorldHandle world);
    void clear();

    [[nodiscard]] RenderObjectManager& getObjectManager(WorldHandle world);
    [[nodiscard]] RenderObjectManager const& getObjectManager(WorldHandle world) const;

    [[nodiscard]] RenderWorld& get(WorldHandle world) { return m_renderWorlds.at(world); }
    [[nodiscard]] const RenderWorld& get(WorldHandle world) const { return m_renderWorlds.at(world); }

private:
    vk::DescriptorPool& m_descriptorPool;
    vk::DescriptorSetLayout& m_descriptorSetLayout;
    std::unordered_map<WorldHandle, RenderWorld> m_renderWorlds;
};
