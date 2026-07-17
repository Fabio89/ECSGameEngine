export module Render.RenderManager;
export import Render.Commands;
import Core;
import Engine.Camera;
import Engine.FrameTimer;
import Geometry;
import Guid;
import Math;
import Render.CommandProcessor;
import Render.EditorCallbacks;
import Render.ImGui;
import Render.RenderObject;
import Render.RenderWorld;
import Render.Viewport;
import Render.Vulkan;
import Render.VulkanResource;
import ThreadSafeQueue;
import Window;
import WorldHandle;

export enum class RenderPipelineType
{
    Opaque,
    Textured,
    Transparent,
};

export class RenderManager
{
public:
    RenderManager();
    RenderManager(const RenderManager&) = delete;
    RenderManager(RenderManager&&) = delete;
    RenderManager& operator=(const RenderManager&) = delete;
    RenderManager& operator=(RenderManager&&) = delete;
    ~RenderManager();

    bool hasBeenInitialized() const { return m_initialised; }
    void init(WindowHandle window);
    void update();
    void shutdown();
    void clear();

    template <typename T>
    void addCommand(T&& command) { m_commandProcessor.getQueue().addCommand(std::forward<T>(command)); }
    RenderCommandQueue& getCommandQueue() { return m_commandProcessor.getQueue(); }

    void updateFramebufferSize();
    float getDeltaTime() const { return m_frameTimer.deltaTime(); }

    void setEditorCallbacks(EditorCallbacks callback);

    ViewportId createViewport(std::span<WorldHandle> worlds, Rect area);

    ViewportManager& viewports() { return m_viewportManager; }
    const ViewportManager& viewports() const { return m_viewportManager; }

private:
    class RenderCommandBase;
    template <typename T>
    class RenderCommand;

    std::mutex m_updateLockMutex;
    EditorCallbacks m_editorCallbacks;

    bool m_initialised{};
    RenderWorldManager m_renderWorldManager;
    ViewportManager m_viewportManager;
    RenderCommandProcessor m_commandProcessor;
    WindowHandle m_window{};
    VulkanContext m_context;
    vk::Queue m_presentQueue{};
    vk::Queue m_transferQueue{};
    vk::DescriptorSetLayout m_descriptorSetLayout{};
    vk::PipelineLayout m_pipelineLayout{};
    vk::Pipeline m_graphicsPipeline{};
    vk::Pipeline m_gizmoPipeline{};
    vk::Pipeline m_linePipeline{};

    vk::CommandPool m_transferCommandPool{};
    std::vector<vk::CommandBuffer> m_commandBuffers;

    vk::PipelineCache m_pipelineCache{};
    vk::DescriptorPool m_descriptorPool{};

    std::vector<vk::Semaphore> m_imageAvailableSemaphores;
    std::vector<vk::Semaphore> m_renderFinishedSemaphores;
    std::vector<vk::Fence> m_inFlightFences;

    vk::DebugUtilsMessengerEXT m_debugMessenger{};

    ImGuiRenderer m_imguiHelper;
    FrameTimer m_frameTimer;
    UInt32 m_currentFrame{0};
    bool m_terminated{};
    std::atomic<bool> m_framebufferResized{false};

    static std::vector<const char*> getRequiredExtensions();

    void createInstance();
    void createSurface();
    void createLogicalDevice();

    void recreateSwapchain();
    void createSwapchain();
    void cleanupSwapchain();

    void createCommandPool();
    void createCommandBuffers();
    void createSyncObjects();
    void pickPhysicalDevice();
    [[nodiscard]] static bool checkValidationLayerSupport();
    void drawFrame();
};
