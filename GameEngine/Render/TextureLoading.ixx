export module Engine.Render.Core:TextureLoading;
import :Vulkan;
import :Utils;

export namespace TextureUtils
{
    // Create a Vulkan image
    [[nodiscard]] std::tuple<vk::Image, vk::DeviceMemory>
    createImage(vk::Device device,
                vk::PhysicalDevice physicalDevice,
                vk::MemoryPropertyFlags properties,
                vk::Extent2D extent,
                vk::Format format,
                vk::ImageTiling tiling,
                vk::ImageUsageFlags usage);

    [[nodiscard]] vk::ImageView createImageView(vk::Device device, vk::Image image, vk::Format format);

    // Create a Vulkan image from the specified file
    [[nodiscard]] std::tuple<vk::Image, vk::DeviceMemory>
    createTextureImage(const char* path, vk::Device device, vk::PhysicalDevice physicalDevice, vk::Queue commandQueue,
                       vk::CommandPool
                       commandPool);

    [[nodiscard]] vk::ImageView createTextureImageView(vk::Device device, vk::Image image);

    [[nodiscard]] vk::Sampler createTextureSampler(vk::Device device, vk::PhysicalDevice physicalDevice);
}
