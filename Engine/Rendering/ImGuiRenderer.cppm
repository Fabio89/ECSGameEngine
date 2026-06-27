export module Render.ImGui;
export import UI.IPanel;
import Core;
import Render.Vulkan;
import Window;

export struct ImGuiInitInfo
{
    vk::Instance instance{};
    vk::PhysicalDevice physicalDevice{};
    vk::Device device{};
    vk::SurfaceKHR surface{};
    vk::Queue queue{};
    vk::RenderPass renderPass{};
    std::size_t imageCount{};
    vk::PipelineCache pipelineCache{};
    vk::Format colorFormat{};
    vk::Format depthFormat{};
};

export class ImGuiRenderer
{
public:
    static constexpr bool enabled = true;
    void init(WindowHandle window, const ImGuiInitInfo& initInfo);

    void beginFrame();
    void renderFrame(vk::CommandBuffer commandBuffer);
    void recreateSwapchain(std::size_t newSize);
    void shutdown();

private:
    vk::Device m_device{};
    vk::DescriptorPool m_descriptorPool{};
};
