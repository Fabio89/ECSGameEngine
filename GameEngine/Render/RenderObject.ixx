export module Engine.Render.Core:RenderObject;
import :Model;
import :Vulkan;
import <glm/glm.hpp>;

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

struct UniformBufferObject
{
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};

struct RenderObject
{
    Mesh mesh;
    Texture texture;
    glm::vec3 location;
    glm::vec3 rotation;
    float scale{1.f};
    std::vector<vk::Buffer> uniformBuffers;
    std::vector<vk::DeviceMemory> uniformBuffersMemory;
    std::vector<void*> uniformBuffersMapped;
    std::vector<vk::DescriptorSet> descriptorSets;
};

export class RenderObjectManager
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

    void createRenderObject(MeshId mesh, TextureId texture, glm::vec3 location = {}, glm::vec3 rotation = {},
                            float scale = 1.f);

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
};
