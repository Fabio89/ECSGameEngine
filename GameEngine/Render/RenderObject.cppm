export module Engine:Render.RenderObject;
import :Core;
import :Render.Model;
import :CoreTypes;
import std;

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
    mat4 model{};
    mat4 view{};
    mat4 proj{};
};

struct RenderObject
{
    Entity entity{};
    Mesh mesh;
    Texture texture;
    vec3 location{};
    vec3 rotation{};
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
        vec3 location{};
        vec3 rotation{};
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

    void clear();
    void addCommand(RenderMessages::AddObject command);
    void addCommand(RenderMessages::SetTransform command);
    void executePendingCommands();
    void addRenderObject(Entity entity, const MeshAsset& meshAsset, const TextureAsset& textureAsset);
    void setObjectTransform(Entity entity, vec3 location = {}, vec3 rotation = {}, float scale = 1.f);
    
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

    vk::Device m_device{};
    vk::PhysicalDevice m_physicalDevice{};
    vk::SurfaceKHR m_surface{};
    vk::DescriptorPool m_descriptorPool{};
    vk::DescriptorSetLayout m_descriptorSetLayout{};
    vk::Queue m_queue{};
    vk::CommandPool m_cmdPool{};
};
