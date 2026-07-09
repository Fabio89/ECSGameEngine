module Render.Image;
import Render.TextureLoading;

Image::Image(VulkanContext& context, const ImageCreateInfo& info): VulkanResource{context}
{
    create(info);
}

Image::~Image()
{
    destroy();
}

void Image::recreate(const ImageCreateInfo& info)
{
    destroy();
    create(info);
}

void Image::create(const ImageCreateInfo& info)
{
    std::tie(m_image, m_memory) = RenderUtils::createImage
    (
        context().device,
        context().physicalDevice,
        info.memoryProperties,
        info.extent,
        info.format,
        info.tiling,
        info.usage
    );

    m_view = RenderUtils::createImageView(context().device, m_image, info.format, info.aspect);
}

void Image::destroy()
{
    if (m_view)
        context().device.destroyImageView(m_view);

    if (m_image)
        context().device.destroyImage(m_image);

    if (m_memory)
        context().device.freeMemory(m_memory);
}