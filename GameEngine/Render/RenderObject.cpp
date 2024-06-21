module;

module Engine.Render.Core;
import std;
import <glm/gtc/matrix_transform.hpp>;

void RenderObjectManager::init
(
    vk::Device device,
    vk::PhysicalDevice physicalDevice,
    vk::SurfaceKHR surface,
    vk::DescriptorPool descriptorPool,
    vk::DescriptorSetLayout descriptorSetLayout,
    vk::Queue queue,
    vk::CommandPool cmdPool
)
{
    m_device = device;
    m_physicalDevice = physicalDevice;
    m_surface = surface;
    m_descriptorPool = descriptorPool;
    m_descriptorSetLayout = descriptorSetLayout;
    m_queue = queue;
    m_cmdPool = cmdPool;
}

TextureId RenderObjectManager::createTexture(const char* path)
{
    static std::atomic<TextureId> currentId{};

    Texture texture;
    texture.id = currentId++;
    std::string fullPath = Config::getContentRoot() + path;
    std::tie(texture.image, texture.memory) = RenderUtils::createTextureImage(
        fullPath.c_str(), m_device, m_physicalDevice,
        m_queue, m_cmdPool);
    texture.view = RenderUtils::createTextureImageView(m_device, texture.image);
    texture.sampler = RenderUtils::createTextureSampler(m_device, m_physicalDevice);
    m_textures.emplace_back(texture);
    return texture.id;
}

Texture RenderObjectManager::getTextureData(TextureId textureId) const
{
    auto it = std::ranges::find_if(m_textures, [textureId](auto&& texture) { return texture.id == textureId; });
    return it != m_textures.end() ? *it : Texture{};
}

void RenderObjectManager::updateDescriptorSets(const RenderObject& object) const
{
    const bool hasTexture = object.texture.image;
    const uint32_t descriptorWriteCount = static_cast<uint32_t>(hasTexture) + 1;

    for (size_t i = 0; i < MaxFramesInFlight; i++)
    {
        std::vector<vk::WriteDescriptorSet> descriptorWrites;
        descriptorWrites.reserve(descriptorWriteCount);

        const vk::DescriptorBufferInfo vertexBufferInfo
        {
            .buffer = object.uniformBuffers[i],
            .offset = 0,
            .range = sizeof(UniformBufferObject),
        };
        descriptorWrites.emplace_back
        (
            vk::WriteDescriptorSet
            {
                .dstSet = object.descriptorSets[i],
                .dstBinding = 0,
                .dstArrayElement = 0,
                .descriptorCount = 1,
                .descriptorType = vk::DescriptorType::eUniformBuffer,
                .pBufferInfo = &vertexBufferInfo,
            }
        );

        if (hasTexture)
        {
            const vk::DescriptorImageInfo imageInfo
            {
                .sampler = object.texture.sampler,
                .imageView = object.texture.view,
                .imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal,
            };
            descriptorWrites.emplace_back
            (
                vk::WriteDescriptorSet
                {
                    .dstSet = object.descriptorSets[i],
                    .dstBinding = 1,
                    .dstArrayElement = 0,
                    .descriptorCount = 1,
                    .descriptorType = vk::DescriptorType::eCombinedImageSampler,
                    .pImageInfo = &imageInfo,
                }
            );
        }

        m_device.updateDescriptorSets
        (
            descriptorWriteCount,
            descriptorWrites.data(),
            0,
            nullptr
        );
    }
}

void RenderObjectManager::updateUniformBuffer(RenderObject& object, vk::Extent2D swapchainExtent, uint32_t currentImage,
                                              float deltaTime)
{
    static float timeElapsed = 0.f;

    //rotate(glm::mat4(1.0f), timeElapsed * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f))
    UniformBufferObject uniformBufferObject
    {
        .model = rotate(glm::mat4(1), glm::radians(object.rotation.x), {1, 0, 0})
            * rotate(glm::mat4(1), glm::radians(object.rotation.y), {0, 1, 0})
            * rotate(glm::mat4(1), glm::radians(object.rotation.z), {0, 0, 1})
            * translate(glm::mat4(1), object.location)
            *scale(glm::mat4(1), glm::vec3{object.scale, object.scale, object.scale}),
        .view = lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
        .proj = glm::perspective(glm::radians(45.0f),
                                 swapchainExtent.width / static_cast<float>(swapchainExtent.height), 0.1f, 10.0f),
    };
    uniformBufferObject.proj[1][1] *= -1;

    memcpy(object.uniformBuffersMapped[currentImage], &uniformBufferObject, sizeof(uniformBufferObject));
    timeElapsed += deltaTime;
}

MeshId RenderObjectManager::createMesh(MeshData data)
{
    assert(m_device);

    Mesh mesh;
    const RenderUtils::CreateDataBufferInfo vertexBufferInfo
    {
        .device = m_device,
        .physicalDevice = m_physicalDevice,
        .surface = m_surface,
        .usageType = vk::BufferUsageFlagBits::eVertexBuffer,
        .transferQueue = m_queue,
        .transferCommandPool = m_cmdPool,
    };
    std::tie(mesh.vertexBuffer, mesh.vertexBufferMemory) = createDataBuffer(data.vertices, vertexBufferInfo);

    const RenderUtils::CreateDataBufferInfo indexBufferInfo
    {
        .device = m_device,
        .physicalDevice = m_physicalDevice,
        .surface = m_surface,
        .usageType = vk::BufferUsageFlagBits::eIndexBuffer,
        .transferQueue = m_queue,
        .transferCommandPool = m_cmdPool,
    };
    std::tie(mesh.indexBuffer, mesh.indexBufferMemory) = createDataBuffer(data.indices, indexBufferInfo);

    mesh.data = std::move(data);

    static std::atomic<MeshId> currentId{};
    const MeshId id = mesh.id = currentId++;
    m_meshes.emplace_back(std::move(mesh));
    return id;
}

Mesh RenderObjectManager::getMeshData(MeshId meshId) const
{
    auto it = std::ranges::find_if(m_meshes, [meshId](auto&& mesh) { return mesh.id == meshId; });
    return it != m_meshes.end() ? *it : Mesh{};
}

void RenderObjectManager::shutdown()
{
    for (const RenderObject& object : m_objects)
    {
        for (auto&& [buffer, memory] : std::views::zip(object.uniformBuffers, object.uniformBuffersMemory))
        {
            m_device.destroyBuffer(buffer);
            m_device.freeMemory(memory);
        }
    }
    m_objects.clear();

    for (const Mesh& mesh : m_meshes)
    {
        m_device.destroyBuffer(mesh.vertexBuffer);
        m_device.freeMemory(mesh.vertexBufferMemory);
        m_device.destroyBuffer(mesh.indexBuffer);
        m_device.freeMemory(mesh.indexBufferMemory);
    }
    m_meshes.clear();

    for (const Texture& texture : m_textures)
    {
        m_device.destroySampler(texture.sampler);
        m_device.destroyImageView(texture.view);
        m_device.destroyImage(texture.image);
        m_device.freeMemory(texture.memory);
    }
    m_textures.clear();
}

void RenderObjectManager::createRenderObject(MeshId mesh, TextureId texture, glm::vec3 location, glm::vec3 rotation, float scale)
{
    RenderObject object;
    static constexpr vk::DeviceSize bufferSize = sizeof(UniformBufferObject);

    object.mesh = getMeshData(mesh);
    object.texture = getTextureData(texture);
    object.location = location;
    object.rotation = rotation;
    object.scale = scale;
    object.uniformBuffers = std::vector<vk::Buffer>(MaxFramesInFlight);
    object.uniformBuffersMemory = std::vector<vk::DeviceMemory>(MaxFramesInFlight);
    object.uniformBuffersMapped = std::vector<void*>(MaxFramesInFlight);

    const RenderUtils::CreateBufferInfo bufferInfo
    {
        .device = m_device,
        .physicalDevice = m_physicalDevice,
        .size = bufferSize,
        .usage = vk::BufferUsageFlagBits::eUniformBuffer,
        .properties = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
    };

    auto bufferRange = std::views::zip(object.uniformBuffers, object.uniformBuffersMemory, object.uniformBuffersMapped);

    for (auto&& [buffer, memory, mapped] : bufferRange)
    {
        std::tie(buffer, memory) = createBuffer(bufferInfo);
        if (m_device.mapMemory(memory, 0, bufferSize, {}, &mapped) != vk::Result::eSuccess)
            return;
    }

    std::vector layouts(MaxFramesInFlight, m_descriptorSetLayout);
    const vk::DescriptorSetAllocateInfo allocInfo
    {
        .descriptorPool = m_descriptorPool,
        .descriptorSetCount = static_cast<uint32_t>(layouts.size()),
        .pSetLayouts = layouts.data(),
    };

    object.descriptorSets = m_device.allocateDescriptorSets(allocInfo);

    m_objects.emplace_back(std::move(object));
}

void RenderObjectManager::renderFrame(vk::CommandBuffer commandBuffer, vk::Pipeline graphicsPipeline,
                                      vk::PipelineLayout pipelineLayout, vk::Extent2D swapchainExtent, float deltaTime,
                                      uint32_t currentFrame)
{
    for (RenderObject& object : m_objects)
    {
        commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, graphicsPipeline);

        updateDescriptorSets(object);

        commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout, 0, 1,
                                         &object.descriptorSets[currentFrame], 0, nullptr);

        const vk::Buffer vertexBuffers[] = {object.mesh.vertexBuffer};
        constexpr vk::DeviceSize offsets[] = {0};

        updateUniformBuffer(object, swapchainExtent, currentFrame, deltaTime);

        commandBuffer.bindVertexBuffers(0, vertexBuffers, offsets);
        commandBuffer.bindIndexBuffer(object.mesh.indexBuffer, 0, MeshData::indexType);
        commandBuffer.drawIndexed(static_cast<uint32_t>(object.mesh.data.indices.size()), 1, 0, 0, 0);
    }
}