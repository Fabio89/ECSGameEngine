export module Render.RenderManager;
export import Render.Commands;
import Core;
import CoreTypes;
import Guid;
import ImGuiHelper;
import Math;
import Render.Model;
import Render.RenderObject;
import Wrapper.Glfw;

export class RenderManager
{
public:
    RenderManager() = default;
    ~RenderManager();
    RenderManager(const RenderManager&) = delete;
    RenderManager(RenderManager&&) = delete;
    RenderManager& operator=(const RenderManager&) = delete;
    RenderManager& operator=(RenderManager&&) = delete;

    bool hasBeenInitialized() const { return m_initialised; }
    void init(GLFWwindow* window);
    void update();
    void shutdown();
    void clear();

    template <typename T>
    void addCommand(T&& command);

    void addDebugWidget(std::unique_ptr<IDebugWidget> widget);
    void setCamera(const Camera& camera);
    float getAspectRatio() const { return m_swapchainExtent.height > 0 ? m_swapchainExtent.width / static_cast<float>(m_swapchainExtent.height) : 1.f; }

    void updateFramebufferSize() { m_framebufferResized = true; }
    float getDeltaTime() const { return m_deltaTime; }

private:
    class RenderCommandBase;
    template <typename T>
    class RenderCommand;

    bool m_initialised{};
    RenderObjectManager m_renderObjectManager;
    ThreadSafeQueue<std::unique_ptr<RenderCommandBase>> m_commands;
    GLFWwindow* m_window{};
    vk::Instance m_instance{};
    vk::PhysicalDevice m_physicalDevice{};
    vk::Device m_device{};
    vk::SurfaceKHR m_surface{};
    vk::SwapchainKHR m_swapChain{};
    vk::Queue m_graphicsQueue{};
    vk::Queue m_presentQueue{};
    vk::Queue m_transferQueue{};
    vk::RenderPass m_renderPass{};
    vk::DescriptorSetLayout m_descriptorSetLayout{};
    vk::PipelineLayout m_pipelineLayout{};
    vk::Pipeline m_graphicsPipeline{};
    vk::Pipeline m_linePipeline{};

    vk::CommandPool m_commandPool{};
    vk::CommandPool m_transferCommandPool{};
    std::vector<vk::CommandBuffer> m_commandBuffers;

    vk::PipelineCache m_pipelineCache{};
    vk::DescriptorPool m_descriptorPool{};

    std::vector<vk::Image> m_swapChainImages;
    std::vector<vk::ImageView> m_swapChainImageViews;
    std::vector<vk::Framebuffer> m_swapChainFramebuffers;
    vk::Format m_swapChainImageFormat{vk::Format::eUndefined};
    vk::Extent2D m_swapchainExtent{0, 0};

    std::vector<vk::Semaphore> m_imageAvailableSemaphores;
    std::vector<vk::Semaphore> m_renderFinishedSemaphores;
    std::vector<vk::Fence> m_inFlightFences;

    vk::Image m_depthImage{};
    vk::DeviceMemory m_depthImageMemory{};
    vk::ImageView m_depthImageView{};

    vk::DebugUtilsMessengerEXT m_debugMessenger{};

    ImGuiHelper m_imguiHelper;
    DeltaTimeTracker m_deltaTime;
    std::atomic<bool> m_framebufferResized{};
    UInt32 m_currentFrame{0};
    bool m_terminated{};

    static std::vector<const char*> getRequiredExtensions();

    void createInstance();
    void createSurface();
    void createLogicalDevice();

    void recreateSwapchain();
    void createSwapchain();
    void createImageViews();
    void createDepthResources();
    void createFramebuffers();

    void cleanupSwapchain() const;

    void createRenderPass();
    void createDescriptorSetLayout();
    void createGraphicsPipeline();
    void createLinePipeline();
    void createCommandPool();
    void createCommandBuffers();
    void createSyncObjects();
    void createDescriptorPool();
    void pickPhysicalDevice();
    [[nodiscard]] static bool checkValidationLayerSupport();
    void drawFrame();

    template <typename T>
    void processCommand(T&& command);
};

class RenderManager::RenderCommandBase
{
public:
    RenderCommandBase() = default;
    virtual ~RenderCommandBase() = default;
    RenderCommandBase(const RenderCommandBase&) = default;
    RenderCommandBase& operator=(const RenderCommandBase&) = default;
    RenderCommandBase(RenderCommandBase&&) = default;
    RenderCommandBase& operator=(RenderCommandBase&&) = default;

    virtual void process()
    {
    }
};

template <typename T>
class RenderManager::RenderCommand : public RenderCommandBase
{
public:
    using RenderCommandBase::RenderCommandBase;

    RenderCommand(RenderManager* renderManager, const T& data) : m_renderManager{renderManager}, m_data{std::forward<T>(data)}
    {
    }

    RenderCommand(RenderManager* renderManager, T&& data) : m_renderManager{renderManager}, m_data{std::forward<T>(data)}
    {
    }

    void process() final { m_renderManager->processCommand<T>(std::forward<T>(m_data)); }

private:
    RenderManager* m_renderManager{};
    T m_data;
};

template <typename T>
void RenderManager::addCommand(T&& command)
{
    m_commands.push(std::make_unique<RenderCommand<T>>(this, std::forward<T>(command)));
}

template <>
void RenderManager::processCommand(RenderCommands::AddMesh&& cmd)
{
    m_renderObjectManager.addMesh(std::move(cmd.data), cmd.guid);
}

template <>
void RenderManager::processCommand(RenderCommands::AddTexture&& cmd)
{
    m_renderObjectManager.addTexture(std::move(cmd.data), cmd.guid);
}

template <>
void RenderManager::processCommand(RenderCommands::AddObject&& cmd)
{
    m_renderObjectManager.addRenderObject(cmd.entity, cmd.mesh, cmd.texture);
}

template <>
void RenderManager::processCommand(RenderCommands::AddLineObject&& cmd)
{
    m_renderObjectManager.addLineRenderObject(cmd.entity, std::move(cmd.vertices));
}

template <>
void RenderManager::processCommand(RenderCommands::SetTransform&& cmd)
{
    m_renderObjectManager.setObjectTransform(cmd.entity, cmd.location, cmd.rotation, cmd.scale);
}

template <>
void RenderManager::processCommand(RenderCommands::SetObjectVisibility&& cmd)
{
    m_renderObjectManager.setObjectVisibility(cmd.entity, cmd.visible);
}
