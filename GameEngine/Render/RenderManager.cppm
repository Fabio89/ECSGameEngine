export module Render.RenderManager;
import Core;
import CoreTypes;
import ImGuiHelper;
import Math;
import Render.IRenderManager;
import Render.Model;
import Render.RenderObject;
import Wrapper.Glfw;

export class RenderManager final : public IRenderManager
{
public:
    RenderManager() = default;
    ~RenderManager() override;
    RenderManager(const RenderManager&) = delete;
    RenderManager(RenderManager&&) = delete;
    RenderManager& operator=(const RenderManager&) = delete;
    RenderManager& operator=(RenderManager&&) = delete;

    bool hasBeenInitialized() const override { return m_initialised; }
    void init(GLFWwindow* window) override;
    void update() override;
    void shutdown() override;
    void clear() override;
    
    void addDebugWidget(std::unique_ptr<IDebugWidget> widget) override;
    void addRenderObject(Entity entity, const MeshAsset* mesh, const TextureAsset* texture) override;
    void setRenderObjectTransform(Entity entity, Vec3 location, Quat rotation, float scale = 1.f) override;
    void setLineRenderObject(Entity entity, const std::vector<LineVertex>& vertices) override;
    void setCamera(const Camera& camera) override;
    float getAspectRatio() const override { return m_swapchainExtent.height > 0 ? m_swapchainExtent.width / static_cast<float>(m_swapchainExtent.height) : 1.f; }

    void updateFramebufferSize() { m_framebufferResized = true; }
    float getDeltaTime() const { return m_deltaTime; }
    
private:
    bool m_initialised{};
    RenderObjectManager m_renderObjectManager;
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
};
