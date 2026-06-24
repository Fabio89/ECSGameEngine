export module ImGuiHelper;
export import DevUI.IDebugWidget;
import Core;
import Wrapper.Glfw;
import Wrapper.Vulkan;

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

export class ImGuiHelper
{
public:
    static constexpr bool enabled = true;
    void init(GLFWwindow* window, const ImGuiInitInfo& initInfo);

    template <typename T>
    void addWidget() { m_debugWidgets.emplace_back(std::make_unique<T>()); }

    void addWidget(std::unique_ptr<IWidget> widget) { m_debugWidgets.emplace_back(std::move(widget)); }

    void drawFrame();
    void renderFrame(vk::CommandBuffer commandBuffer);
    void recreateSwapchain(std::size_t newSize);
    void shutdown();

private:
    vk::Device m_device{};
    vk::DescriptorPool m_descriptorPool{};
    std::vector<std::unique_ptr<IWidget>> m_debugWidgets;
};
