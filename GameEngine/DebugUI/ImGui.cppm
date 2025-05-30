export module ImGuiHelper;
export import DebugUI.IDebugWidget;
import Core;
import Wrapper.Glfw;
import Wrapper.Vulkan;

export struct ImGuiInitInfo
{
    vk::Instance instance;
    vk::PhysicalDevice physicalDevice;
    vk::Device device;
    vk::SurfaceKHR surface;
    vk::Queue queue;
    vk::RenderPass renderPass;
    size_t imageCount;
    vk::PipelineCache pipelineCache;
};

export class ImGuiHelper
{
public:
    static constexpr bool enabled = false;
    void init(GLFWwindow* window, const ImGuiInitInfo& initInfo);

    template <typename T>
    void addWidget() { m_debugWidgets.emplace_back(std::make_unique<T>()); }

    void addWidget(std::unique_ptr<IDebugWidget> widget) { m_debugWidgets.emplace_back(std::move(widget)); }
    
    void drawFrame();
    void renderFrame(vk::CommandBuffer commandBuffer);
    void shutdown();

private:
    vk::Device m_device{};
    vk::DescriptorPool m_descriptorPool{};
    std::vector<std::unique_ptr<IDebugWidget>> m_debugWidgets;
};
