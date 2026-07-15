module Render.Viewport;
import Core;
import Render.Utils;
import Math;

Viewport::Viewport(ViewportId id, VulkanContext& context, ViewportCreateInfo&& info)
    : VulkanResource{context},
      m_id{id},
      m_renderWorlds{std::move(info.renderWorlds)},
      m_requestedArea{info.requestedArea},
      m_extent{static_cast<UInt32>(info.requestedArea.size.width), static_cast<UInt32>(info.requestedArea.size.height)},
      m_offset{info.requestedArea.position.x, info.requestedArea.position.y},
      m_color{context, makeColorImageInfo()},
      m_depth{context, makeDepthImageInfo()} {}

void Viewport::recreate()
{
    m_colorLayout = vk::ImageLayout::eUndefined;
    m_depthLayout = vk::ImageLayout::eUndefined;

    if (m_extent.width == 0 || m_extent.height == 0)
        return;

    m_color.recreate(makeColorImageInfo());
    m_depth.recreate(makeDepthImageInfo());
}

void Viewport::update()
{
    if (m_requestedArea.size.width != m_extent.width
        || m_requestedArea.size.height != m_extent.height
        || m_requestedArea.position.x != m_offset.x
        || m_requestedArea.position.y != m_offset.y)
    {
        const auto [position, size] = m_requestedArea;
        m_offset = vk::Offset2D{position.x, position.y};
        m_extent = vk::Extent2D{static_cast<UInt32>(size.width), static_cast<UInt32>(size.height)};

        recreate();
    }
}

void Viewport::drawFrame(const RenderPassContext& renderContext)
{
    //--------------------------------------------------------------------------
    // Render scene into offscreen viewport image
    //--------------------------------------------------------------------------
    const vk::RenderingAttachmentInfo colorAttachmentInfo
    {
        .imageView = m_color.getView(),
        .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
        .loadOp = vk::AttachmentLoadOp::eClear,
        .storeOp = vk::AttachmentStoreOp::eStore,
        .clearValue = vk::ClearColorValue{0.f, 0.f, 0.f, 1.f}
    };

    const vk::RenderingAttachmentInfo depthAttachmentInfo
    {
        .imageView = m_depth.getView(),
        .imageLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal,
        .loadOp = vk::AttachmentLoadOp::eClear,
        .storeOp = vk::AttachmentStoreOp::eDontCare,
        .clearValue = vk::ClearDepthStencilValue{1.f, 0}
    };

    const vk::RenderingInfo renderingInfo
    {
        .renderArea = {{0,0}, m_extent},
        .layerCount = 1,
        .colorAttachmentCount = 1,
        .pColorAttachments = &colorAttachmentInfo,
        .pDepthAttachment = &depthAttachmentInfo,
    };

    if (m_color.getImage())
    {
        const IVec2 srcOffset
        {
            std::max(0, -m_offset.x),
            std::max(0, -m_offset.y)
        };

        const IVec2 dstOffset
        {
            std::max(0,  m_offset.x),
            std::max(0,  m_offset.y)
        };

        const vk::Extent2D swapchainExtent = context().swapchain.extent;

        const Size2D size
        {
            std::min(static_cast<int>(m_extent.width) - srcOffset.x, static_cast<int>(swapchainExtent.width) - dstOffset.x),
            std::min(static_cast<int>(m_extent.height) - srcOffset.y, static_cast<int>(swapchainExtent.height) - dstOffset.y)
        };

        if (size.width > 0 && size.height > 0)
        {
            RenderUtils::transitionImageLayout
            (
                renderContext.commandBuffer,
                m_color.getImage(),
                m_colorLayout,
                vk::ImageLayout::eColorAttachmentOptimal,
                {},
                vk::AccessFlagBits::eColorAttachmentWrite,
                vk::PipelineStageFlagBits::eTopOfPipe,
                vk::PipelineStageFlagBits::eColorAttachmentOutput
            );
            m_colorLayout = vk::ImageLayout::eColorAttachmentOptimal;

            RenderUtils::transitionImageLayout
            (
                renderContext.commandBuffer,
                m_depth.getImage(),
                m_depthLayout,
                vk::ImageLayout::eDepthStencilAttachmentOptimal,
                {},
                vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite,
                vk::PipelineStageFlagBits::eTopOfPipe,
                vk::PipelineStageFlagBits::eEarlyFragmentTests | vk::PipelineStageFlagBits::eLateFragmentTests,
                vk::ImageAspectFlagBits::eDepth
            );
            m_depthLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

            renderContext.commandBuffer.beginRendering(renderingInfo);

            const vk::Viewport viewport
            {
                .x = 0,
                .y = 0,
                .width = static_cast<float>(m_extent.width),
                .height = static_cast<float>(m_extent.height),
                .minDepth = 0.f,
                .maxDepth = 1.f,
            };

            renderContext.commandBuffer.setViewport(0, 1, &viewport);

            const vk::Rect2D scissor
            {
                .offset = {0,0},
                .extent = m_extent,
            };

            renderContext.commandBuffer.setScissor(0, 1, &scissor);

            for (auto& world : m_renderWorlds)
                world.get().drawFrame(renderContext);

            renderContext.commandBuffer.endRendering();

            //------------------------------------------------------------------
            // Copy viewport into swapchain
            //------------------------------------------------------------------

            RenderUtils::transitionImageLayout
            (
                renderContext.commandBuffer,
                m_color.getImage(),
                m_colorLayout,
                vk::ImageLayout::eTransferSrcOptimal,
                vk::AccessFlagBits::eColorAttachmentWrite,
                vk::AccessFlagBits::eTransferRead,
                vk::PipelineStageFlagBits::eColorAttachmentOutput,
                vk::PipelineStageFlagBits::eTransfer
            );
            m_colorLayout = vk::ImageLayout::eTransferSrcOptimal;

            RenderUtils::transitionImageLayout
            (
                renderContext.commandBuffer,
                context().swapchain.images[renderContext.imageIndex],
                context().swapchain.layouts[renderContext.imageIndex],
                vk::ImageLayout::eTransferDstOptimal,
                vk::AccessFlagBits::eColorAttachmentWrite,
                vk::AccessFlagBits::eTransferWrite,
                vk::PipelineStageFlagBits::eColorAttachmentOutput,
                vk::PipelineStageFlagBits::eTransfer
            );
            context().swapchain.layouts[renderContext.imageIndex] = vk::ImageLayout::eTransferDstOptimal;

            renderContext.commandBuffer.copyImage
            (
                m_color.getImage(),
                vk::ImageLayout::eTransferSrcOptimal,
                context().swapchain.images[renderContext.imageIndex],
                vk::ImageLayout::eTransferDstOptimal,
                vk::ImageCopy
                {
                    .srcSubresource = {.aspectMask = vk::ImageAspectFlagBits::eColor, .layerCount = 1},
                    .srcOffset = {srcOffset.x, srcOffset.y, 0},
                    .dstSubresource = {.aspectMask = vk::ImageAspectFlagBits::eColor, .layerCount = 1},
                    .dstOffset = {dstOffset.x, dstOffset.y, 0},
                    .extent = {static_cast<UInt32>(size.width), static_cast<UInt32>(size.height), 1}
                }
            );
        }
    }
}

void Viewport::setArea(Rect area)
{
    m_requestedArea = area;
}

Rect Viewport::getArea() const
{
    return m_requestedArea;
}

float Viewport::getAspectRatio() const
{
    return m_extent.height > 0 ? m_extent.width / static_cast<float>(m_extent.height) : 1.f;
}

void Viewport::setCamera(Camera camera)
{
    m_camera = std::move(camera);
}

const Camera& Viewport::getCamera() const
{
    return m_camera;
}

ImageCreateInfo Viewport::makeColorImageInfo() const
{
    return {
        .extent = m_extent,
        .format = context().swapchain.imageFormat,
        .usage = vk::ImageUsageFlagBits::eColorAttachment |
                 vk::ImageUsageFlagBits::eSampled |
                 vk::ImageUsageFlagBits::eTransferSrc,
        .aspect = vk::ImageAspectFlagBits::eColor
    };
}

ImageCreateInfo Viewport::makeDepthImageInfo() const
{
    return {
        .extent = m_extent,
        .format = RenderUtils::findDepthFormat(context().physicalDevice),
        .usage = vk::ImageUsageFlagBits::eDepthStencilAttachment,
        .aspect = vk::ImageAspectFlagBits::eDepth
    };
}

ViewportId ViewportManager::createViewport(ViewportCreateInfo&& info)
{
    const ViewportId id{m_nextId++};
    m_viewports.try_emplace(id, id, context(), std::move(info));
    return id;
}

void ViewportManager::setViewportArea(ViewportId id, Rect area)
{
    m_viewports.at(id).setArea(area);
}

Rect ViewportManager::getViewportArea(ViewportId id) const
{
    return m_viewports.at(id).getArea();
}

float ViewportManager::getAspectRatio(ViewportId id) const
{
    return m_viewports.at(id).getAspectRatio();
}

const Camera& ViewportManager::getCamera(ViewportId id) const
{
    return m_viewports.at(id).getCamera();
}

void ViewportManager::setCamera(ViewportId id, const Camera& camera)
{
    m_viewports.at(id).setCamera(camera);
}

void ViewportManager::update()
{
    for (Viewport& viewport : m_viewports | std::views::values)
        viewport.update();
}

void ViewportManager::drawViewports(const RenderPassContext& renderContext)
{
    for (Viewport& viewport : m_viewports | std::views::values)
        viewport.drawFrame(renderContext);
}

void ViewportManager::recreateImages()
{
    for (Viewport& viewport : m_viewports | std::views::values)
        viewport.recreate();
}

void ViewportManager::shutdown()
{
    m_viewports.clear();
}