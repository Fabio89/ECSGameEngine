module;

#define STB_IMAGE_IMPLEMENTATION
#include <External/TextureLoading/stb_image.h>

module Engine.Render.Core;
import std;

// Create a Vulkan image
[[nodiscard]] std::tuple<vk::Image, vk::DeviceMemory>
TextureUtils::createImage(vk::Device device,
                          vk::PhysicalDevice physicalDevice,
                          vk::MemoryPropertyFlags properties,
                          vk::Extent2D extent,
                          vk::Format format,
                          vk::ImageTiling tiling,
                          vk::ImageUsageFlags usage)
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
        throw std::runtime_error("failed to create image!");
    }

    const vk::MemoryRequirements memRequirements = device.getImageMemoryRequirements(image);
    const vk::MemoryAllocateInfo allocInfo
    {
        .allocationSize = memRequirements.size,
        .memoryTypeIndex = RenderUtils::findMemoryType(physicalDevice, memRequirements.memoryTypeBits, properties)
    };

    vk::DeviceMemory memory = device.allocateMemory(allocInfo);
    if (!memory)
    {
        throw std::runtime_error("failed to allocate image memory!");
    }

    device.bindImageMemory(image, memory, 0);

    return std::make_pair(image, memory);
}

// Create a Vulkan image from the specified file
[[nodiscard]] std::tuple<vk::Image, vk::DeviceMemory>
TextureUtils::createTextureImage(const char* path, vk::Device device, vk::PhysicalDevice physicalDevice,
                                 vk::Queue commandQueue, vk::CommandPool
                                 commandPool)
{
    int texWidth, texHeight, texChannels;
    stbi_uc* pixels = stbi_load(path, &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    const vk::DeviceSize imageSize = texWidth * texHeight * 4;

    if (!pixels)
        throw std::runtime_error("failed to load texture image!");

    const RenderUtils::CreateBufferInfo bufferInfo
    {
        .device = device,
        .physicalDevice = physicalDevice,
        .size = imageSize,
        .usage = vk::BufferUsageFlagBits::eTransferSrc,
        .properties = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
    };

    auto&& [stagingBuffer, stagingBufferMemory] = createBuffer(bufferInfo);

    void* data = device.mapMemory(stagingBufferMemory, 0, imageSize);
    memcpy(data, pixels, imageSize);
    device.unmapMemory(stagingBufferMemory);

    stbi_image_free(pixels);

    const vk::Extent2D imageExtent{static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight)};

    auto result = createImage
    (
        device,
        physicalDevice,
        vk::MemoryPropertyFlagBits::eDeviceLocal,
        imageExtent,
        vk::Format::eR8G8B8A8Srgb,
        vk::ImageTiling::eOptimal,
        vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled
    );

    const auto image = std::get<vk::Image>(result);

    constexpr auto format = vk::Format::eR8G8B8A8Srgb;
    RenderUtils::transitionImageLayout(device, commandQueue, commandPool,
                                       image, format, vk::ImageLayout::eUndefined,
                                       vk::ImageLayout::eTransferDstOptimal);

    RenderUtils::copyBufferToImage(device, commandQueue, commandPool, stagingBuffer, image, imageExtent);

    RenderUtils::transitionImageLayout(device, commandQueue, commandPool,
                                       image, format, vk::ImageLayout::eTransferDstOptimal,
                                       vk::ImageLayout::eShaderReadOnlyOptimal);

    device.destroyBuffer(stagingBuffer);
    device.freeMemory(stagingBufferMemory);

    return result;
}
