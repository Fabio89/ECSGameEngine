export module Engine.Render.Application;

import Engine.Render.Core;
import std;
import <glm/glm.hpp>;

constexpr size_t MaxFramesInFlight{2};

struct Texture
{
    TextureId id;
    vk::Image image;
    vk::DeviceMemory memory;
    vk::ImageView view;
    vk::Sampler sampler; // TODO: should be shared between textures??
};

struct Mesh
{
    MeshId id;
    MeshData data;
    vk::Buffer vertexBuffer{nullptr};
    vk::DeviceMemory vertexBufferMemory{nullptr};
    vk::Buffer indexBuffer{nullptr};
    vk::DeviceMemory indexBufferMemory{nullptr};
};

struct RenderObject
{
    Mesh mesh;
    Texture texture;
    glm::vec3 location;
    
    std::vector<vk::Buffer> uniformBuffers;
    std::vector<vk::DeviceMemory> uniformBuffersMemory;
    std::vector<void*> uniformBuffersMapped;
    std::vector<vk::DescriptorSet> descriptorSets;
};

class RenderObjectManager
{
public:
    void init
    (
        vk::Device device,
        vk::PhysicalDevice physicalDevice,
        vk::SurfaceKHR surface,
        vk::DescriptorPool descriptorPool,
        vk::DescriptorSetLayout descriptorSetLayout,
        vk::Queue queue,
        vk::CommandPool cmdPool
    );
    
    void shutdown();

    void createRenderObject(MeshId mesh, TextureId texture, glm::vec3 location);

    MeshId createMesh(MeshData data);

    TextureId createTexture(const char* path);

    void renderFrame
    (
        vk::CommandBuffer commandBuffer,
        vk::Pipeline graphicsPipeline,
        vk::PipelineLayout pipelineLayout,
        vk::Extent2D swapchainExtent,
        float deltaTime,
        uint32_t currentFrame
    );

private:
    Mesh getMeshData(MeshId meshId) const;
    Texture getTextureData(TextureId textureId) const;

    void updateDescriptorSets(const RenderObject& object) const;
    static void updateUniformBuffer(RenderObject& object, vk::Extent2D swapchainExtent, uint32_t currentImage,
                                    float deltaTime);

    std::vector<RenderObject> m_objects;
    std::vector<Mesh> m_meshes;
    std::vector<Texture> m_textures;
    vk::Device m_device{nullptr};
    vk::PhysicalDevice m_physicalDevice{nullptr};
    vk::SurfaceKHR m_surface{nullptr};
    vk::DescriptorPool m_descriptorPool{nullptr};
    vk::DescriptorSetLayout m_descriptorSetLayout{nullptr};
    vk::Queue m_queue{nullptr};
    vk::CommandPool m_cmdPool{nullptr};
} objectManager;

struct UniformBufferObject
{
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};

struct ImGuiInitInfo
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

class ImGuiHelper
{
public:
    static constexpr bool enabled = true;
    void init(GLFWwindow* window, const ImGuiInitInfo& initInfo);
    void drawFrame();
    void renderFrame(vk::CommandBuffer commandBuffer);
    void shutdown();

private:
    bool m_showDemoWindow{true};
    vk::Device m_device{nullptr};
    vk::DescriptorPool m_descriptorPool{nullptr};
};

export class VulkanApplication
{
public:
    ~VulkanApplication() noexcept;
    void init();
    void update(float deltaTime);
    void shutdown();
    bool shouldWindowClose() const;

private:
    glm::ivec2 m_windowSize{1600, 900};
    GLFWwindow* m_window{nullptr};
    vk::Instance m_instance{nullptr};
    vk::PhysicalDevice m_physicalDevice{nullptr};
    vk::Device m_device{nullptr};
    vk::SurfaceKHR m_surface{nullptr};
    vk::SwapchainKHR m_swapChain{nullptr};
    vk::Queue m_graphicsQueue{nullptr};
    vk::Queue m_presentQueue{nullptr};
    vk::Queue m_transferQueue{nullptr};
    vk::RenderPass m_renderPass{nullptr};
    vk::DescriptorSetLayout m_descriptorSetLayout{nullptr};
    vk::PipelineLayout m_pipelineLayout{nullptr};
    vk::Pipeline m_graphicsPipeline{nullptr};

    vk::CommandPool m_commandPool{nullptr};
    vk::CommandPool m_transferCommandPool{nullptr};
    std::vector<vk::CommandBuffer> m_commandBuffers;

    vk::PipelineCache m_pipelineCache{nullptr};
    vk::DescriptorPool m_descriptorPool{nullptr};

    std::vector<vk::Image> m_swapChainImages;
    std::vector<vk::ImageView> m_swapChainImageViews;
    std::vector<vk::Framebuffer> m_swapChainFramebuffers;
    vk::Format m_swapChainImageFormat{vk::Format::eUndefined};
    vk::Extent2D m_swapchainExtent{0, 0};

    std::vector<vk::Semaphore> m_imageAvailableSemaphores;
    std::vector<vk::Semaphore> m_renderFinishedSemaphores;
    std::vector<vk::Fence> m_inFlightFences;

    vk::Image m_depthImage{nullptr};
    vk::DeviceMemory m_depthImageMemory{nullptr};
    vk::ImageView m_depthImageView{nullptr};
    
    vk::DebugUtilsMessengerEXT m_debugMessenger{nullptr};

    ImGuiHelper m_imguiHelper;

    uint32_t m_currentFrame{0};
    bool m_framebufferResized{false};
    bool m_terminated{false};
    static std::vector<const char*> getRequiredExtensions();
    static void framebufferResizeCallback(GLFWwindow* window, int width, int height);

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
