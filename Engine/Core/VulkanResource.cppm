export module Render.VulkanResource;
export import Core;
export import Render.Vulkan;

export struct RenderPipelineSet
{
    vk::Pipeline mesh{};
    vk::Pipeline gizmo{};
    vk::Pipeline line{};
    vk::PipelineLayout layout{};
};

export struct PresentationImage
{
    vk::Image image{};
    vk::ImageView view{};
    vk::ImageLayout* layout{};
    vk::Extent2D extent{};
    vk::Format format{};
};

export struct RenderPassContext
{
    vk::CommandBuffer commandBuffer{};
    const RenderPipelineSet& pipelines;
    Int32 frameIndex{};
    Int32 imageIndex{};
    PresentationImage destination;
};

export struct VulkanContext
{
    vk::Instance instance{};
    vk::PhysicalDevice physicalDevice{};
    vk::Device device{};
    vk::CommandPool commandPool{};
    vk::Queue graphicsQueue{};
    vk::SurfaceKHR surface{};
};

export class VulkanResource
{
public:
    explicit VulkanResource(VulkanContext& context) : m_context{context} {}
    VulkanResource(const VulkanResource&) = delete;
    VulkanResource(VulkanResource&&) = delete;

protected:
    [[nodiscard]] VulkanContext& context() { return m_context; }
    [[nodiscard]] const VulkanContext& context() const { return m_context; }

private:
    VulkanContext& m_context;
};