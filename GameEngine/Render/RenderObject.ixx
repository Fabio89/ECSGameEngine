export module Engine.Render.Core:RenderObject;
import :Model;
import :Vulkan;
import Engine.Core;
import Engine.Guid;
import Engine.Render;
import std;
import <glm/glm.hpp>;

struct Texture
{
    TextureId id{};
    vk::Image image{};
    vk::DeviceMemory memory{};
    vk::ImageView view{};
    vk::Sampler sampler{}; // TODO: should be shared between textures??
};

struct Mesh
{
    MeshId id{};
    MeshData data;
    vk::Buffer vertexBuffer{};
    vk::DeviceMemory vertexBufferMemory{};
    vk::Buffer indexBuffer{};
    vk::DeviceMemory indexBufferMemory{};
};

struct UniformBufferObject
{
    glm::mat4 model{};
    glm::mat4 view{};
    glm::mat4 proj{};
};

struct RenderObject
{
    Entity entity{};
    Mesh mesh;
    Texture texture;
    glm::vec3 location{};
    glm::vec3 rotation{};
    float scale{1.f};
    std::vector<vk::Buffer> uniformBuffers;
    std::vector<vk::DeviceMemory> uniformBuffersMemory;
    std::vector<void*> uniformBuffersMapped;
    std::vector<vk::DescriptorSet> descriptorSets;
};

export namespace RenderMessages
{
    struct AddObject
    {
        Entity entity;
        const MeshAsset* mesh{};
        const TextureAsset* texture{};
    };

    struct SetTransform
    {
        Entity entity;
        glm::vec3 location{};
        glm::vec3 rotation{};
        float scale{1.f};
    };
}

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
    void addCommand(RenderMessages::AddObject command);
    void addCommand(RenderMessages::SetTransform command);
    void executePendingCommands();
    void addRenderObject(Entity entity, const MeshAsset& meshAsset, const TextureAsset& textureAsset);
    void setObjectTransform(Entity entity, glm::vec3 location = {}, glm::vec3 rotation = {}, float scale = 1.f);

    const Mesh& addMesh(MeshData data, Guid guid);
    const Texture& addTexture(const TextureData& textureData, Guid guid);

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
    void updateDescriptorSets(const RenderObject& object) const;
    static void updateUniformBuffer(RenderObject& object, vk::Extent2D swapchainExtent, uint32_t currentImage,
                                    float deltaTime);

    ThreadSafeQueue<RenderMessages::AddObject> m_addObjectCommands;
    ThreadSafeQueue<RenderMessages::SetTransform> m_setTransformCommands;

    std::vector<RenderObject> m_objects;
    std::vector<Mesh> m_meshes;
    std::vector<Texture> m_textures;

    std::unordered_map<Guid, MeshId> m_meshMap;
    std::unordered_map<Guid, TextureId> m_textureMap;

    vk::Device m_device{nullptr};
    vk::PhysicalDevice m_physicalDevice{nullptr};
    vk::SurfaceKHR m_surface{nullptr};
    vk::DescriptorPool m_descriptorPool{nullptr};
    vk::DescriptorSetLayout m_descriptorSetLayout{nullptr};
    vk::Queue m_queue{nullptr};
    vk::CommandPool m_cmdPool{nullptr};
};
