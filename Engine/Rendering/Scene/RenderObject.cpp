module Render.RenderObject;
import AssetManager;
import Render.TextureLoading;
import Render.Utils;

template<typename T>
auto findRenderObject(std::vector<T>& span, Entity entity)
{
    return std::ranges::find_if(span, [&](auto&& object) { return object.entity == entity; });
}

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
    m_objects.reserve(100);
    m_meshes.reserve(100);
    m_textures.reserve(100);
}

void RenderObjectManager::clear()
{
    m_meshMap.clear();
    m_textureMap.clear();

    for (auto& span : m_objects | std::views::values)
        for (auto& object : span)
            removeRenderObject(object);
    m_objects.clear();

    for (const LineRenderObject& lineObject : m_lineObjects)
        removeLineRenderObject(lineObject);
    m_lineObjects.clear();

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

void RenderObjectManager::setCamera(const Camera& camera)
{
    m_camera = std::move(camera);
}

void RenderObjectManager::addRenderObject(Entity entity, const MeshData* mesh, const TextureData* texture, Mat4 transform, RenderLayer layer, Vec4 tint)
{
    RenderObject object
    {
        .entity = entity,
        .mesh = getOrCreateMesh(mesh),
        .texture = getOrCreateTexture(texture),
        .layer = layer,
        .tint = std::move(tint),
        .model = std::move(transform),
        .uniformBuffers = std::vector<vk::Buffer>(MaxFramesInFlight),
        .uniformBuffersMemory = std::vector<vk::DeviceMemory>(MaxFramesInFlight),
        .uniformBuffersMapped = std::vector<void*>(MaxFramesInFlight),
    };

    static constexpr vk::DeviceSize bufferSize = sizeof(UniformBufferObject);

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
        .descriptorSetCount = static_cast<UInt32>(layouts.size()),
        .pSetLayouts = layouts.data(),
    };

    object.descriptorSets = m_device.allocateDescriptorSets(allocInfo);

    m_objects[layer].emplace_back(std::move(object));
}

void RenderObjectManager::removeRenderObject(Entity entity)
{
    for (auto& span : m_objects | std::views::values)
    {
        if (auto it = findRenderObject(span, entity); it != span.end())
        {
            removeRenderObject(*it);
            span.erase(it);
        }
    }
}

void RenderObjectManager::setObjectTransform(Entity entity, const Mat4& worldTransform)
{
    for (auto& span : m_objects | std::views::values)
    {
        if (auto it = findRenderObject(span, entity); it != span.end())
        {
            it->model = worldTransform;
        }
    }

    if (auto it = findRenderObject(m_lineObjects, entity); it != m_lineObjects.end())
    {
        it->model = worldTransform;
    }
}

void RenderObjectManager::addLineRenderObject(Entity entity, std::vector<LineVertex>&& vertices, Mat4 transform)
{
    check(!std::ranges::any_of(m_lineObjects, [&](const LineRenderObject& object) { return object.entity == entity; }), "Added a line render object more than once!");

    LineRenderObject object;

    const RenderUtils::CreateDataBufferInfo vertexBufferInfo
    {
        .device = m_device,
        .physicalDevice = m_physicalDevice,
        .surface = m_surface,
        .usageType = vk::BufferUsageFlagBits::eVertexBuffer,
        .transferQueue = m_queue,
        .transferCommandPool = m_cmdPool,
    };

    object.entity = entity;
    object.vertices = std::move(vertices);
    object.model = std::move(transform);
    object.uniformBuffers = std::vector<vk::Buffer>(MaxFramesInFlight);
    object.uniformBuffersMemory = std::vector<vk::DeviceMemory>(MaxFramesInFlight);
    object.uniformBuffersMapped = std::vector<void*>(MaxFramesInFlight);

    std::tie(object.vertexBuffer, object.vertexBufferMemory) = createDataBuffer(object.vertices, vertexBufferInfo);

    static constexpr vk::DeviceSize bufferSize = sizeof(UniformBufferObject);

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
        .descriptorSetCount = static_cast<UInt32>(layouts.size()),
        .pSetLayouts = layouts.data(),
    };

    object.descriptorSets = m_device.allocateDescriptorSets(allocInfo);

    m_lineObjects.emplace_back(std::move(object));
    log(std::format("Added debug render object for entity '{}'", entity));
}

void RenderObjectManager::removeLineRenderObject(Entity entity)
{
    if (auto it = findRenderObject(m_lineObjects, entity); it != m_lineObjects.end())
    {
        removeLineRenderObject(*it);
        m_lineObjects.erase(it);
    }
}

void RenderObjectManager::setObjectVisibility(Entity entity, bool visible)
{
    for (auto& span : m_objects | std::views::values)
    {
        if (auto it = findRenderObject(span, entity); it != span.end())
        {
            it->visible = visible;
            return;
        }
    }

    if (auto it = findRenderObject(m_lineObjects, entity); it != m_lineObjects.end())
    {
        it->visible = visible;
    }
}

void RenderObjectManager::renderFrame
(
    RenderLayer layer,
    vk::CommandBuffer commandBuffer,
    vk::PipelineLayout pipelineLayout,
    UInt32 currentFrame
)
{
    auto it = m_objects.find(layer);
    if (it == m_objects.end())
        return;

    for (RenderObject& object : it->second)
    {
        if (!object.visible)
            continue;

        updateDescriptorSets(object);

        commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout, 0, 1,
            &object.descriptorSets[currentFrame], 0, nullptr);

        updateUniformBuffer(object, currentFrame);

        const Mesh& mesh = m_meshes[object.mesh];
        constexpr vk::DeviceSize offsets[] = {0};

        commandBuffer.bindVertexBuffers(0, {mesh.vertexBuffer}, offsets);
        commandBuffer.bindIndexBuffer(mesh.indexBuffer, 0, MeshData::indexType);
        commandBuffer.drawIndexed(mesh.indexCount, 1, 0, 0, 0);
    }
}

void RenderObjectManager::renderLineFrame(vk::CommandBuffer commandBuffer, vk::PipelineLayout pipelineLayout, UInt32 currentFrame)
{
    for (LineRenderObject& object : m_lineObjects)
    {
        if (!object.visible)
            continue;

        RenderUtils::updateBuffer(object.vertices, m_device, object.vertexBufferMemory); // TODO(perf): Shouldn't need to do this every frame

        updateLineDescriptorSets(object);

        commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout, 0, 1,
                                         &object.descriptorSets[currentFrame], 0, nullptr);

        const vk::Buffer vertexBuffers[] = {object.vertexBuffer};
        constexpr vk::DeviceSize offsets[] = {0};

        updateUniformBuffer(object, currentFrame);
        commandBuffer.bindVertexBuffers(0, vertexBuffers, offsets);
        commandBuffer.draw(static_cast<UInt32>(object.vertices.size()), 1, 0, 0);
    }
}

std::size_t RenderObjectManager::getOrCreateMesh(const MeshData* mesh)
{
    if (!check(mesh, "[RenderObjectManager] Tried to use null mesh!")
        || !check(mesh->vertices.size() > 0, "[RenderObjectManager] Tried to add mesh with no vertices", ErrorType::Warning))
        return std::numeric_limits<std::size_t>::max();

    if (auto it = m_meshMap.find(mesh); it != m_meshMap.end())
        return it->second;

    Mesh& renderMesh = m_meshes.emplace_back();
    renderMesh.id = m_meshes.size() - 1;

    const RenderUtils::CreateDataBufferInfo vertexBufferInfo
    {
        .device = m_device,
        .physicalDevice = m_physicalDevice,
        .surface = m_surface,
        .usageType = vk::BufferUsageFlagBits::eVertexBuffer,
        .transferQueue = m_queue,
        .transferCommandPool = m_cmdPool,
    };
    std::tie(renderMesh.vertexBuffer, renderMesh.vertexBufferMemory) = createDataBuffer(mesh->vertices, vertexBufferInfo);

    const RenderUtils::CreateDataBufferInfo indexBufferInfo
    {
        .device = m_device,
        .physicalDevice = m_physicalDevice,
        .surface = m_surface,
        .usageType = vk::BufferUsageFlagBits::eIndexBuffer,
        .transferQueue = m_queue,
        .transferCommandPool = m_cmdPool,
    };
    std::tie(renderMesh.indexBuffer, renderMesh.indexBufferMemory) = createDataBuffer(mesh->indices, indexBufferInfo);

    renderMesh.indexCount = mesh->indices.size();

    m_meshMap.emplace(mesh, renderMesh.id);

    return renderMesh.id;
}

std::size_t RenderObjectManager::getOrCreateTexture(const TextureData* texture)
{
    if (auto it = m_textureMap.find(texture); it != m_textureMap.end())
        return it->second;

    if (!texture)
    {
        static const TextureData whiteTexture
        {
            .size = {1, 1},
            .format = vk::Format::eR8G8B8A8Srgb,
            .pixels = {255, 255, 255, 255}
        };
        texture = &whiteTexture;
    }

    const TextureData& data = *texture;

    Texture& renderTexture = m_textures.emplace_back();
    renderTexture.id = m_textures.size() - 1;
    std::tie(renderTexture.image, renderTexture.memory) = RenderUtils::createTextureImage(data, m_device, m_physicalDevice, m_queue, m_cmdPool);

    if (renderTexture.image)
    {
        renderTexture.view = RenderUtils::createTextureImageView(m_device, renderTexture.image);
        renderTexture.sampler = RenderUtils::createTextureSampler(m_device, m_physicalDevice);
        m_textureMap.emplace(texture, renderTexture.id);
    }

    return renderTexture.id;
}

void RenderObjectManager::updateDescriptorSets(const RenderObject& object) const
{
    const Texture* texture = m_textures.size() > object.texture ? &m_textures.at(object.texture) : nullptr;
    const bool hasTexture = texture && texture->image;
    const UInt32 descriptorWriteCount = static_cast<UInt32>(hasTexture) + 1;

    for (std::size_t i = 0; i < MaxFramesInFlight; i++)
    {
        std::array<vk::DescriptorBufferInfo, 1> bufferInfos;
        std::array<vk::DescriptorImageInfo, 1> imageInfos;
        std::array<vk::WriteDescriptorSet, 2> descriptorWrites;

        bufferInfos[0] = vk::DescriptorBufferInfo
        {
            .buffer = object.uniformBuffers[i],
            .offset = 0,
            .range = sizeof(UniformBufferObject),
        };

        descriptorWrites[0] = vk::WriteDescriptorSet
        {
            .dstSet = object.descriptorSets[i],
            .dstBinding = 0,
            .descriptorCount = 1,
            .descriptorType = vk::DescriptorType::eUniformBuffer,
            .pBufferInfo = &bufferInfos[0],
        };

        if (hasTexture)
        {
            imageInfos[0] = vk::DescriptorImageInfo
            {
                .sampler = texture->sampler,
                .imageView = texture->view,
                .imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal,
            };

            descriptorWrites[1] = vk::WriteDescriptorSet
            {
                .dstSet = object.descriptorSets[i],
                .dstBinding = 1,
                .descriptorCount = 1,
                .descriptorType = vk::DescriptorType::eCombinedImageSampler,
                .pImageInfo = &imageInfos[0],
            };
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

// ReSharper disable once CppParameterMayBeConstPtrOrRef
void RenderObjectManager::updateUniformBuffer(RenderObject& object, UInt32 currentImage)
{
    UniformBufferObject uniformBufferObject
    {
        .model = object.model,
        .view = m_camera.view,
        .proj = m_camera.proj,
        .tint = object.tint
    };

    std::memcpy(object.uniformBuffersMapped[currentImage], &uniformBufferObject, sizeof(uniformBufferObject));
}

void RenderObjectManager::updateUniformBuffer(LineRenderObject& object, UInt32 currentImage)
{
    UniformBufferObject uniformBufferObject
    {
        .model = object.model,
        .view = m_camera.view,
        .proj = m_camera.proj,
    };

    std::memcpy(object.uniformBuffersMapped[currentImage], &uniformBufferObject, sizeof(uniformBufferObject));
}

void RenderObjectManager::updateLineDescriptorSets(const LineRenderObject& object) const
{
    for (std::size_t i = 0; i < MaxFramesInFlight; i++)
    {
        const vk::DescriptorBufferInfo vertexBufferInfo
        {
            .buffer = object.uniformBuffers[i],
            .offset = 0,
            .range = sizeof(UniformBufferObject),
        };

        std::vector descriptorWrites
        {
            vk::WriteDescriptorSet
            {
                .dstSet = object.descriptorSets[i],
                .dstBinding = 0,
                .dstArrayElement = 0,
                .descriptorCount = 1,
                .descriptorType = vk::DescriptorType::eUniformBuffer,
                .pBufferInfo = &vertexBufferInfo,
            }
        };

        m_device.updateDescriptorSets
        (
            static_cast<UInt32>(descriptorWrites.size()),
            descriptorWrites.data(),
            0,
            nullptr
        );
    }
}

void RenderObjectManager::removeRenderObject(const RenderObject& object)
{
    for (auto&& [buffer, memory] : std::views::zip(object.uniformBuffers, object.uniformBuffersMemory))
    {
        m_device.destroyBuffer(buffer);
        m_device.freeMemory(memory);
    }

    check(std::in_range<UInt32>(object.descriptorSets.size()), "Truncating value of object.descriptorSets.size() when freeing descriptor sets!");
    auto result = m_device.freeDescriptorSets(m_descriptorPool, static_cast<UInt32>(object.descriptorSets.size()), object.descriptorSets.data());
    check(result == vk::Result::eSuccess, "[RenderObjectManager::clear] Failed to free descriptor sets!");
}

void RenderObjectManager::removeLineRenderObject(const LineRenderObject& object)
{
    for (auto&& [buffer, memory] : std::views::zip(object.uniformBuffers, object.uniformBuffersMemory))
    {
        m_device.destroyBuffer(buffer);
        m_device.freeMemory(memory);
    }
    m_device.destroyBuffer(object.vertexBuffer);
    m_device.freeMemory(object.vertexBufferMemory);

    check(std::in_range<UInt32>(object.descriptorSets.size()), "Truncating value of object.descriptorSets.size() when freeing descriptor sets!");
    auto result = m_device.freeDescriptorSets(m_descriptorPool, static_cast<UInt32>(object.descriptorSets.size()), object.descriptorSets.data());
    check(result == vk::Result::eSuccess, "[RenderObjectManager::clear] Failed to free descriptor sets!");
}
