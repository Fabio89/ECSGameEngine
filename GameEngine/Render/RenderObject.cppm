export module Engine:Render.RenderObject;
import :Ecs;
import :Render.Model;
import :CoreTypes;

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
    Mat4 model{};
    Mat4 view{};
    Mat4 proj{};
};

struct Camera
{
    Vec3 location{};
    Quat rotation{};
    float fov{60.f};
    float nearPlane{0.1f};
    float farPlane{10.0f};
};

struct RenderObject
{
    Entity entity{};
    Mesh mesh;
    Texture texture;
    Vec3 location{};
    Quat rotation{};
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
        Vec3 location{};
        Quat rotation{};
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
    void setObjectTransform(Entity entity, Vec3 location = {}, Quat rotation = {}, float scale = 1.f);
    void setCameraTransform(Vec3 location = {}, Quat rotation = {});
    void setCameraFov(float fov);
    void setAspectRatio(float aspectRatio);

    const Mesh& addMesh(MeshData data, Guid guid);
    const Texture& addTexture(const TextureData& textureData, Guid guid);

    void renderFrame
    (
        vk::CommandBuffer commandBuffer,
        vk::Pipeline graphicsPipeline,
        vk::PipelineLayout pipelineLayout,
        float deltaTime,
        uint32_t currentFrame
    );

private:
    void updateDescriptorSets(const RenderObject& object) const;
    void updateUniformBuffer(RenderObject& object, uint32_t currentImage, float deltaTime);

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
    Camera m_camera{};
    Mat4 m_view{};
    Mat4 m_proj{};
    float m_aspectRatio{1.f};
};
