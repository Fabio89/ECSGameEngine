export module Render.Image;
import Core;
import Render.VulkanResource;

export struct ImageCreateInfo
{
    vk::Extent2D extent{};
    vk::Format format{};
    vk::ImageUsageFlags usage{};
    vk::ImageAspectFlags aspect{};
    vk::ImageType type{vk::ImageType::e2D};
    vk::ImageTiling tiling{vk::ImageTiling::eOptimal};
    vk::MemoryPropertyFlags memoryProperties{vk::MemoryPropertyFlagBits::eDeviceLocal};
    vk::SampleCountFlagBits samples{vk::SampleCountFlagBits::e1};
    vk::ImageLayout initialLayout{vk::ImageLayout::eUndefined};
    Int32 mipLevels{1};
    Int32 arrayLayers{1};
};

export class Image : VulkanResource
{
public:
    explicit Image(VulkanContext& context, const ImageCreateInfo& info);
    ~Image();

    [[nodiscard]] vk::Image getImage() const { return m_image; }
    [[nodiscard]] vk::ImageView getView() const { return m_view; }
    void recreate(const ImageCreateInfo& info);

private:
    void create(const ImageCreateInfo& info);
    void destroy();

    vk::Image m_image{};
    vk::DeviceMemory m_memory{};
    vk::ImageView m_view{};
};