export module Engine:Render.RenderManager;
import :IDebugWidget;
import :ImGui;
import :Render.IRenderManager;
import :Render.RenderObject;
import :Math;
import std;

export class RenderManager final : public IRenderManager
{
public:
    RenderManager() = default;
    ~RenderManager() override;
    RenderManager(const RenderManager&) = delete;
    RenderManager(RenderManager&&) = delete;
    RenderManager& operator=(const RenderManager&) = delete;
    RenderManager& operator=(RenderManager&&) = delete;

    bool hasBeenInitialised() const override { return m_initialised; }
    void init(GLFWwindow* window) override;
    void update(float deltaTime) override;
    void shutdown() override;

    void addDebugWidget(std::unique_ptr<IDebugWidget> widget) override;
    void addRenderObject(Entity entity, const MeshAsset* mesh, const TextureAsset* texture) override;
    void setRenderObjectTransform(Entity entity, vec3 location, vec3 rotation, float scale = 1.f) override;

    void updateFramebufferSize() { m_framebufferResized = true; }

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

    std::atomic<bool> m_framebufferResized{};
    uint32_t m_currentFrame{0};
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
    void createCommandPool();
    void createCommandBuffers();
    void createSyncObjects();
    void createDescriptorPool();
    void pickPhysicalDevice();
    [[nodiscard]] static bool checkValidationLayerSupport();
    void drawFrame(float deltaTime);
};
