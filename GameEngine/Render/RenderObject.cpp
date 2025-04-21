module Render.RenderObject;
import Project;
import Render.TextureLoading;
import Render.Utils;

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

void RenderObjectManager::updateDescriptorSets(const RenderObject& object) const
{
    const Texture* texture = m_textures.size() > object.texture ? &m_textures.at(object.texture) : nullptr;
    const bool hasTexture = texture && texture->image;
    const UInt32 descriptorWriteCount = static_cast<UInt32>(hasTexture) + 1;

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
                .sampler = texture->sampler,
                .imageView = texture->view,
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

// ReSharper disable once CppParameterMayBeConstPtrOrRef
void RenderObjectManager::updateUniformBuffer(RenderObject& object, UInt32 currentImage)
{
    UniformBufferObject uniformBufferObject
    {
        .model = object.model,
        .view = m_camera.view,
        .proj = m_camera.proj,
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

void RenderObjectManager::addMesh(MeshData&& data, Guid guid)
{
    check(m_device, "[RenderObjectManager::addMesh] Device is null");
    if (!check(data.vertices.size() > 0, "[RenderObjectManager::addMesh] Tried to add mesh with no vertices", ErrorType::Warning))
    {
        return;
    }

    Mesh& mesh = m_meshes.emplace_back();
    mesh.id = m_meshes.size() - 1;
    
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

    m_meshMap.emplace(guid, mesh.id);
    std::cout << "Added mesh: " << guid << std::endl;
}

void RenderObjectManager::addTexture(TextureData&& textureData, Guid guid)
{
    Texture& texture = m_textures.emplace_back();
    texture.id = m_textures.size() - 1;
    const std::string fullPath = Project::getContentRoot() + std::move(textureData.path);
    std::tie(texture.image, texture.memory) = RenderUtils::createTextureImage(
        fullPath.c_str(), m_device, m_physicalDevice,
        m_queue, m_cmdPool);

    if (texture.image)
    {
        texture.view = RenderUtils::createTextureImageView(m_device, texture.image);
        texture.sampler = RenderUtils::createTextureSampler(m_device, m_physicalDevice);
        m_textureMap.emplace(guid, texture.id);
        std::cout << "Added texture: " << guid << std::endl;
    }
}

void RenderObjectManager::clear()
{
    m_meshMap.clear();
    m_textureMap.clear();

    for (const RenderObject& object : m_objects)
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
    m_objects.clear();

    for (const LineRenderObject& debugObject : m_lineObjects)
    {
        for (auto&& [buffer, memory] : std::views::zip(debugObject.uniformBuffers, debugObject.uniformBuffersMemory))
        {
            m_device.destroyBuffer(buffer);
            m_device.freeMemory(memory);
        }
        m_device.destroyBuffer(debugObject.vertexBuffer);
        m_device.freeMemory(debugObject.vertexBufferMemory);
        
        check(std::in_range<UInt32>(debugObject.descriptorSets.size()), "Truncating value of object.descriptorSets.size() when freeing descriptor sets!");
        auto result = m_device.freeDescriptorSets(m_descriptorPool, static_cast<UInt32>(debugObject.descriptorSets.size()), debugObject.descriptorSets.data());
        check(result == vk::Result::eSuccess, "[RenderObjectManager::clear] Failed to free descriptor sets!");
    }
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

void RenderObjectManager::addRenderObject(Entity entity, Guid meshAsset, const Guid textureAsset)
{
    RenderObject object;
    static constexpr vk::DeviceSize bufferSize = sizeof(UniformBufferObject);

    if (const auto idIt = m_meshMap.find(meshAsset); idIt != m_meshMap.end())
    {
        object.mesh = m_meshes.at(idIt->second).id;
    }
    else
    {
        fatalError("Tried to add a render object with invalid mesh!");
        return;
    }

    if (textureAsset)
    {
        if (const auto idIt = m_textureMap.find(textureAsset); idIt != m_textureMap.end())
        {
            object.texture = m_textures.at(idIt->second).id;
        }
        else
        {
            fatalError("Tried to add a render object with invalid texture even if one was specified!");
            return;
        }
    }
    
    object.entity = entity;
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
        .descriptorSetCount = static_cast<UInt32>(layouts.size()),
        .pSetLayouts = layouts.data(),
    };

    object.descriptorSets = m_device.allocateDescriptorSets(allocInfo);

    m_objects.emplace_back(std::move(object));
    std::cout << "Added render object\n\tMesh: " << meshAsset << std::endl;
    std::cout << "\tTexture: " << textureAsset << std::endl;
}

void RenderObjectManager::setObjectTransform(Entity entity, const Mat4& worldTransform)
{
    if (const auto it = std::ranges::find_if(m_objects,
       [&](auto&& object) { return object.entity == entity; }); it != m_objects.end())
    {
        it->model = worldTransform;
    }

    if (const auto it = std::ranges::find_if(m_lineObjects,
        [&](auto&& object){ return object.entity == entity; }); it != m_lineObjects.end())
    {
        it->model = worldTransform;
    }   
}

void RenderObjectManager::addLineRenderObject(Entity entity, std::vector<LineVertex>&& vertices)
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

void RenderObjectManager::setObjectVisibility(Entity entity, bool visible)
{
    if (const auto it = std::ranges::find_if(m_objects, [&](auto&& object) { return object.entity == entity; });
        it != m_objects.end())
    {
        it->visible = visible;
        return;
    }
    if (const auto it = std::ranges::find_if(m_lineObjects, [&](auto&& object){ return object.entity == entity; });
        it != m_lineObjects.end())
    {
        it->visible = visible;
    }
}

void RenderObjectManager::setCamera(const Camera& camera)
{
    m_camera = std::move(camera);
}

void RenderObjectManager::renderFrame
(
    vk::CommandBuffer commandBuffer,
    vk::PipelineLayout pipelineLayout,
    UInt32 currentFrame
)
{
    for (RenderObject& object : m_objects)
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
        commandBuffer.drawIndexed(static_cast<UInt32>(mesh.data.indices.size()), 1, 0, 0, 0);
    }
}

void RenderObjectManager::updateLineDescriptorSets(const LineRenderObject& object) const
{
    for (size_t i = 0; i < MaxFramesInFlight; i++)
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

void RenderObjectManager::renderLineFrame(vk::CommandBuffer commandBuffer, vk::PipelineLayout pipelineLayout, UInt32 currentFrame)
{
    for (LineRenderObject& object : m_lineObjects)
    {
        if (!object.visible)
            continue;
        
        RenderUtils::updateBuffer(object.vertices, m_device, object.vertexBufferMemory);
        
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
