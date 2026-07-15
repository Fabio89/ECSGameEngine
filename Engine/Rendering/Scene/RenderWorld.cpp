module Render.RenderWorld;

RenderWorld::RenderWorld(VulkanContext& context, const RenderWorldCreateInfo& info)
    : VulkanResource{context},
      m_world{info.world}
{
    m_objects.init(context.device, context.physicalDevice, context.surface, info.descriptorPool, info.descriptorSetLayout, context.graphicsQueue, context.commandPool);
}

RenderWorld::~RenderWorld()
{
    m_objects.clear();
}

void RenderWorld::drawFrame(const RenderPassContext& renderContext)
{
    renderContext.commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, renderContext.pipelines.mesh);
    m_objects.renderFrame(renderContext.commandBuffer, renderContext.pipelines.layout, renderContext.frameIndex);

    renderContext.commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, renderContext.pipelines.line);
    m_objects.renderLineFrame(renderContext.commandBuffer, renderContext.pipelines.layout, renderContext.frameIndex);
}

RenderObjectManager& RenderWorld::objects() { return m_objects; }

const RenderObjectManager& RenderWorld::objects() const { return m_objects; }

RenderWorldManager::RenderWorldManager(VulkanContext& vulkanContext, const RenderWorldManagerContext& worldManagerContext)
    : VulkanResource{vulkanContext},
      m_descriptorPool{worldManagerContext.descriptorPool},
      m_descriptorSetLayout{worldManagerContext.descriptorSetLayout} {}

void RenderWorldManager::registerWorld(WorldHandle world)
{
    if (m_renderWorlds.contains(world))
    {
        report(std::format("Tried to register render world more than once!"));
        return;
    }

    m_renderWorlds.try_emplace(world, context(), RenderWorldCreateInfo{.world = world, .descriptorPool = m_descriptorPool, .descriptorSetLayout = m_descriptorSetLayout});
}

void RenderWorldManager::unregisterWorld(WorldHandle world)
{
    if (!m_renderWorlds.contains(world))
    {
        report(std::format("Tried to unregister render world that wasn't registered before!"));
        return;
    }

    m_renderWorlds.erase(world);
}

RenderObjectManager& RenderWorldManager::getObjectManager(WorldHandle world)
{
    return const_cast<RenderObjectManager&>(std::as_const(*this).getObjectManager(world));
}

RenderObjectManager const& RenderWorldManager::getObjectManager(WorldHandle world) const
{
    return m_renderWorlds.at(world).objects();
}

void RenderWorldManager::clear()
{
    m_renderWorlds.clear();
}
