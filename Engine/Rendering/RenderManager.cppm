export module Render.RenderManager;
export import Render.Commands;
import Core;
import CoreTypes;
import Engine.FrameTimer;
import Geometry;
import Guid;
import Math;
import Render.EditorCallbacks;
import Render.ImGui;
import Render.Vulkan;
import Render.Model;
import Render.RenderObject;
import Window;

export enum class RenderPipelineType
{
    Opaque,
    Textured,
    Transparent,
};

struct RenderTarget
{
    vk::Image image{};
    vk::DeviceMemory memory{};
    vk::ImageView view{};
    vk::Extent2D extent{1000, 800};
    vk::Offset2D offset{};
    vk::ImageLayout layout{vk::ImageLayout::eUndefined};
};

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
    void init(WindowHandle window);
    void update();
    void shutdown();
    void clear();

    template <typename T>
    void addCommand(T&& command);

    void setCamera(const Camera& camera);

    void updateFramebufferSize();
    float getDeltaTime() const { return m_frameTimer.deltaTime(); }

    void setEditorCallbacks(EditorCallbacks callback);

    void setViewportArea(Rect area);
    Rect getViewportArea() const;

    float getViewportAspectRatio() const;

private:
    RenderTarget m_sceneViewport;
    Rect m_requestedViewportArea;

    class RenderCommandBase;
    template <typename T>
    class RenderCommand;

    std::mutex m_updateLockMutex;
    EditorCallbacks m_editorCallbacks;

    bool m_initialised{};
    RenderObjectManager m_renderObjectManager;
    ThreadSafeQueue<std::unique_ptr<RenderCommandBase>> m_commands;
    WindowHandle m_window{};
    vk::Instance m_instance{};
    vk::PhysicalDevice m_physicalDevice{};
    vk::Device m_device{};
    vk::SurfaceKHR m_surface{};
    vk::SwapchainKHR m_swapChain{};
    vk::Queue m_graphicsQueue{};
    vk::Queue m_presentQueue{};
    vk::Queue m_transferQueue{};
    vk::DescriptorSetLayout m_descriptorSetLayout{};
    vk::PipelineLayout m_pipelineLayout{};
    vk::Pipeline m_graphicsPipeline{};
    vk::Pipeline m_linePipeline{};

    vk::CommandPool m_commandPool{};
    vk::CommandPool m_transferCommandPool{};
    std::vector<vk::CommandBuffer> m_commandBuffers;

    vk::PipelineCache m_pipelineCache{};
    vk::DescriptorPool m_descriptorPool{};

    std::vector<vk::Image> m_swapchainImages;
    std::vector<vk::ImageLayout> m_swapchainLayouts;
    std::vector<vk::ImageView> m_swapchainImageViews;
    vk::Format m_swapChainImageFormat{vk::Format::eUndefined};
    vk::Extent2D m_swapchainExtent{0, 0};

    std::vector<vk::Semaphore> m_imageAvailableSemaphores;
    std::vector<vk::Semaphore> m_renderFinishedSemaphores;
    std::vector<vk::Fence> m_inFlightFences;

    vk::Image m_depthImage{};
    vk::DeviceMemory m_depthImageMemory{};
    vk::ImageView m_depthImageView{};

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

    void recreateViewport();
    void updateViewport();
    void cleanupViewport();

    void recreateSwapchain();
    void createSwapchain();
    void createImageViews();
    void createDepthResources();
    void cleanupSwapchain();

    void createCommandPool();
    void createCommandBuffers();
    void createSyncObjects();
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
class RenderManager::RenderCommand final : public RenderCommandBase
{
public:
    RenderCommand(RenderManager* renderManager, T&& data) : m_renderManager{renderManager}, m_data{std::forward<T>(data)}
    {
    }

    void process() override;

private:
    RenderManager* m_renderManager{};
    T m_data;
};

template <typename T>
void RenderManager::addCommand(T&& command)
{
    m_commands.push(std::make_unique<RenderCommand<T>>(this, std::forward<T>(command)));
}

template <typename T>
void RenderManager::RenderCommand<T>::process()
{
    m_renderManager->processCommand<T>(std::forward<T>(m_data));
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
void RenderManager::processCommand(RenderCommands::RemoveObject&& cmd)
{
    m_renderObjectManager.removeRenderObject(cmd.entity);
}

template <>
void RenderManager::processCommand(RenderCommands::AddLineObject&& cmd)
{
    m_renderObjectManager.addLineRenderObject(cmd.entity, std::move(cmd.vertices));
}

template <>
void RenderManager::processCommand(RenderCommands::RemoveLineObject&& cmd)
{
    m_renderObjectManager.removeLineRenderObject(cmd.entity);
}

template <>
void RenderManager::processCommand(RenderCommands::SetTransform&& cmd)
{
    m_renderObjectManager.setObjectTransform(cmd.entity, cmd.worldTransform);
}

template <>
void RenderManager::processCommand(RenderCommands::SetObjectVisibility&& cmd)
{
    m_renderObjectManager.setObjectVisibility(cmd.entity, cmd.visible);
}

template <>
void RenderManager::processCommand(RenderCommands::ClearRenderObjects&&)
{
    clear();
}
