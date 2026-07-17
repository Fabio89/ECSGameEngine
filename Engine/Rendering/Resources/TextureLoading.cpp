module Render.TextureLoading;
import Core;
import Render.Utils;
import Log;

// Create a Vulkan image
std::tuple<vk::Image, vk::DeviceMemory> RenderUtils::createImage
(
    vk::Device device,
    vk::PhysicalDevice physicalDevice,
    vk::MemoryPropertyFlags properties,
    vk::Extent2D extent,
    vk::Format format,
    vk::ImageTiling tiling,
    vk::ImageUsageFlags usage
)
{
    const vk::ImageCreateInfo imageInfo
    {
        .imageType = vk::ImageType::e2D,
        .format = format,
        .extent = {.width = extent.width, .height = extent.height, .depth = 1},
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = vk::SampleCountFlagBits::e1,
        .tiling = tiling,
        .usage = usage,
        .sharingMode = vk::SharingMode::eExclusive,
        .initialLayout = vk::ImageLayout::eUndefined,
    };

    vk::Image image = device.createImage(imageInfo);
    if (!image)
    {
        fatalError("failed to create image!");
    }

    const vk::MemoryRequirements memRequirements = device.getImageMemoryRequirements(image);
    const vk::MemoryAllocateInfo allocInfo
    {
        .allocationSize = memRequirements.size,
        .memoryTypeIndex = findMemoryType(physicalDevice, memRequirements.memoryTypeBits, properties)
    };

    vk::DeviceMemory memory = device.allocateMemory(allocInfo);
    if (!memory)
    {
        fatalError("failed to allocate image memory!");
    }

    device.bindImageMemory(image, memory, 0);

    return std::make_pair(image, memory);
}

vk::ImageView RenderUtils::createImageView
(
    vk::Device device,
    vk::Image image,
    vk::Format format,
    vk::ImageAspectFlags aspectFlags
)
{
    const vk::ImageViewCreateInfo viewInfo
    {
        .image = image,
        .viewType = vk::ImageViewType::e2D,
        .format = format,
        .subresourceRange
        {
            .aspectMask = aspectFlags,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1,
        }
    };
    return device.createImageView(viewInfo);
}

std::tuple<vk::Image, vk::DeviceMemory> RenderUtils::createTextureImage
(
    const TextureData& data,
    vk::Device device,
    vk::PhysicalDevice physicalDevice,
    vk::Queue commandQueue,
    vk::CommandPool commandPool
)
{
    const vk::DeviceSize imageSize = data.size.width * data.size.height * 4;

    const CreateBufferInfo bufferInfo
    {
        .device = device,
        .physicalDevice = physicalDevice,
        .size = imageSize,
        .usage = vk::BufferUsageFlagBits::eTransferSrc,
        .properties = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
    };

    auto&& [stagingBuffer, stagingBufferMemory] = createBuffer(bufferInfo);

    void* memory = device.mapMemory(stagingBufferMemory, 0, imageSize);
    std::memcpy(memory, data.pixels.data(), imageSize);
    device.unmapMemory(stagingBufferMemory);

    const vk::Extent2D imageExtent{narrow_cast<UInt32>(data.size.width), narrow_cast<UInt32>(data.size.height)};

    auto result = createImage
    (
        device,
        physicalDevice,
        vk::MemoryPropertyFlagBits::eDeviceLocal,
        imageExtent,
        data.format,
        vk::ImageTiling::eOptimal,
        vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled
    );

    const auto image = std::get<vk::Image>(result);

    transitionImageLayout(device, commandQueue, commandPool, image, data.format, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);

    copyBufferToImage(device, commandQueue, commandPool, stagingBuffer, image, imageExtent);

    transitionImageLayout(device, commandQueue, commandPool, image, data.format, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);

    device.destroyBuffer(stagingBuffer);
    device.freeMemory(stagingBufferMemory);

    return result;
}

vk::ImageView RenderUtils::createTextureImageView(vk::Device device, vk::Image image)
{
    return createImageView(device, image, vk::Format::eR8G8B8A8Srgb);
}

vk::Sampler RenderUtils::createTextureSampler(vk::Device device, vk::PhysicalDevice physicalDevice)
{
    const vk::PhysicalDeviceProperties properties = physicalDevice.getProperties();

    const vk::SamplerCreateInfo samplerInfo
    {
        .magFilter = vk::Filter::eLinear,
        .minFilter = vk::Filter::eLinear,
        .mipmapMode = vk::SamplerMipmapMode::eLinear,
        .addressModeU = vk::SamplerAddressMode::eRepeat,
        .addressModeV = vk::SamplerAddressMode::eRepeat,
        .addressModeW = vk::SamplerAddressMode::eRepeat,
        .mipLodBias = 0.0f,
        .anisotropyEnable = vk::True,
        .maxAnisotropy = properties.limits.maxSamplerAnisotropy,
        .compareEnable = vk::False,
        .compareOp = vk::CompareOp::eAlways,
        .minLod = 0.0f,
        .maxLod = 0.0f,
        .borderColor = vk::BorderColor::eIntOpaqueBlack,
        .unnormalizedCoordinates = vk::False,
    };

    return device.createSampler(samplerInfo);
}
