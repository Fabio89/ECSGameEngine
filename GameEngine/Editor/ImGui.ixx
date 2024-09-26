module;
export module Engine.ImGui;
import Engine.ApplicationState;
import Engine.DebugWidget;
import Engine.Render.Core;
import Engine.World;
import std;

export struct ImGuiInitInfo
{
    World* world{};
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
    static constexpr bool enabled = true;
    void init(GLFWwindow* window, const ImGuiInitInfo& initInfo);

    template <typename T>
    void addWidget() { m_debugWidgets.emplace_back(std::make_unique<T>()); }

    void addWidget(std::unique_ptr<DebugWidget> widget) { m_debugWidgets.emplace_back(std::move(widget)); }
    
    void drawFrame();
    void renderFrame(vk::CommandBuffer commandBuffer);
    void shutdown();

private:
    World* m_world{};
    vk::Device m_device{};
    vk::DescriptorPool m_descriptorPool{};
    std::vector<std::unique_ptr<DebugWidget>> m_debugWidgets;
};
