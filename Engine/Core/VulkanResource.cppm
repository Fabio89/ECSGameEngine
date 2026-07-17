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

export struct RenderPassContext
{
    vk::CommandBuffer commandBuffer{};
    const RenderPipelineSet& pipelines;
    Int32 frameIndex{};
    Int32 imageIndex{};
};

export struct Swapchain
{
    vk::SwapchainKHR handle{};
    vk::Format imageFormat{vk::Format::eUndefined};
    vk::Extent2D extent{0, 0};
    std::vector<vk::Image> images;
    std::vector<vk::ImageLayout> layouts;
    std::vector<vk::ImageView> imageViews;
};

export struct VulkanContext
{
    vk::Instance instance{};
    vk::PhysicalDevice physicalDevice{};
    vk::Device device{};
    vk::CommandPool commandPool{};
    vk::Queue graphicsQueue{};
    vk::SurfaceKHR surface{};
    Swapchain swapchain;
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