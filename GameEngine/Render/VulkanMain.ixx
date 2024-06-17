export module Engine.Render.Application;

import Engine.Render.Core;
import std;
import <glm/glm.hpp>;

constexpr size_t MaxFramesInFlight{2};

struct Vertex
{
    glm::vec2 pos;
    glm::vec3 color;
    glm::vec2 texCoordinates;

    static vk::VertexInputBindingDescription getBindingDescription();
    static std::array<vk::VertexInputAttributeDescription, 3> getAttributeDescriptions();
};

struct Mesh
{
    struct VertexData
    {
        const std::vector<Vertex> vertices
        {
            {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
            {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
            {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
            {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}}
        };
        std::vector<uint16_t> indices
        {
            0, 1, 2, 2, 3, 0
        };
    } vertexData;
    
    vk::Buffer vertexBuffer{nullptr};
    vk::DeviceMemory vertexBufferMemory{nullptr};
    vk::Buffer indexBuffer{nullptr};
    vk::DeviceMemory indexBufferMemory{nullptr};
    std::vector<vk::Buffer> uniformBuffers;
    std::vector<vk::DeviceMemory> uniformBuffersMemory;
    std::vector<void*> uniformBuffersMapped;
};

[[nodiscard]] Mesh createMesh(Mesh::VertexData data, vk::Device device, vk::PhysicalDevice physicalDevice,
                vk::SurfaceKHR surface, vk::Queue queue,
                vk::CommandPool cmdPool);

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

struct Texture
{
    vk::Image m_image;
    vk::DeviceMemory m_memory;
    vk::ImageView m_view;
    vk::Sampler m_sampler; // TODO: should be shared between textures??
};

Texture createTexture(const char* path, vk::Device device, vk::PhysicalDevice physicalDevice, vk::Queue queue,
                      vk::CommandPool commandPool);
void destroyTexture(vk::Device device, const Texture& texture);

struct RenderObject
{
    Mesh m_mesh;
    Texture m_texture;
};

std::vector<RenderObject> testObjects;

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
    std::vector<vk::DescriptorSet> m_descriptorSets;

    std::vector<vk::Image> m_swapChainImages;
    std::vector<vk::ImageView> m_swapChainImageViews;
    std::vector<vk::Framebuffer> m_swapChainFramebuffers;
    vk::Format m_swapChainImageFormat{vk::Format::eUndefined};
    vk::Extent2D m_swapChainExtent{0, 0};

    std::vector<vk::Semaphore> m_imageAvailableSemaphores;
    std::vector<vk::Semaphore> m_renderFinishedSemaphores;
    std::vector<vk::Fence> m_inFlightFences;

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
    void createFramebuffers();

    void cleanupSwapchain() const;

    void createRenderPass();
    void createDescriptorSetLayout();
    void createGraphicsPipeline();
    void createCommandPool();
    void createCommandBuffers();
    void createSyncObjects();
    void createDescriptorPool();
    void createDescriptorSets();
    void updateDescriptorSets(const RenderObject& object) const;
    void pickPhysicalDevice();
    [[nodiscard]] static bool checkValidationLayerSupport();
    void recordCommandBuffer(vk::CommandBuffer commandBuffer, uint32_t imageIndex);
    void updateUniformBuffer(Mesh& mesh, uint32_t currentImage, float deltaTime) const;
    void drawFrame(float deltaTime);
};
