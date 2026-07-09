export module Render.Viewport;
export import Engine.Viewport;
import Core;
import Geometry;
import Render.Image;
import Render.RenderWorld;
import Render.VulkanResource;

export struct ViewportCreateInfo
{
    Rect requestedArea;
    RenderWorld& renderWorld;
};

export class Viewport : VulkanResource
{
public:
    Viewport(ViewportId id, VulkanContext& context, const ViewportCreateInfo& info);
    void recreate();
    void update();
    void drawFrame(const RenderPassContext& renderContext);
    void setArea(Rect area);
    [[nodiscard]] Rect getArea() const;
    [[nodiscard]] float getAspectRatio() const;

private:
    [[nodiscard]] ImageCreateInfo makeColorImageInfo() const;
    [[nodiscard]] ImageCreateInfo makeDepthImageInfo() const;

    ViewportId m_id;
    RenderWorld& m_renderWorld;
    Rect m_requestedArea;
    vk::Extent2D m_extent{1000, 800};
    vk::Offset2D m_offset{};
    vk::ImageLayout m_colorLayout{vk::ImageLayout::eUndefined};
    vk::ImageLayout m_depthLayout{vk::ImageLayout::eUndefined};
    Image m_color;
    Image m_depth;
};

export class ViewportManager : VulkanResource
{
public:
    using VulkanResource::VulkanResource;

    ViewportId createViewport(const ViewportCreateInfo& info);
    void setViewportArea(ViewportId id, Rect area);
    [[nodiscard]] Rect getViewportArea(ViewportId id) const;
    [[nodiscard]] float getFakeAspectRatio() const;

    void update();
    void drawViewports(const RenderPassContext& renderContext);
    void recreateImages();
    void shutdown();

private:
    std::unordered_map<ViewportId, Viewport> m_viewports;
    ViewportId::ValueType m_nextId{};
};