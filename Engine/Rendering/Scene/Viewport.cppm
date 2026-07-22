export module Render.Viewport;
export import Engine.Viewport;
import Core;
import Engine.Camera;
import Geometry;
import Render.Image;
import Render.RenderWorld;
import Render.VulkanResource;

export struct ViewportCreateInfo
{
    Rect requestedArea;
    vk::Format colorFormat;
    std::vector<std::reference_wrapper<RenderWorld>> renderWorlds;
};

export class Viewport : VulkanResource
{
public:
    Viewport(ViewportId id, VulkanContext& context, ViewportCreateInfo&& info);
    void recreate();
    void update();
    void drawFrame(const RenderPassContext& renderContext);
    void setArea(Rect area);
    [[nodiscard]] Rect getArea() const;
    [[nodiscard]] float getAspectRatio() const;
    void setCamera(Camera camera);
    const Camera& getCamera() const;

private:
    [[nodiscard]] ImageCreateInfo makeColorImageInfo() const;
    [[nodiscard]] ImageCreateInfo makeDepthImageInfo() const;

    ViewportId m_id;
    std::vector<std::reference_wrapper<RenderWorld>> m_renderWorlds;
    Camera m_camera;
    Rect m_requestedArea;
    vk::Extent2D m_extent{1000, 800};
    vk::Offset2D m_offset{};
    vk::ImageLayout m_colorLayout{vk::ImageLayout::eUndefined};
    vk::ImageLayout m_depthLayout{vk::ImageLayout::eUndefined};
    vk::Format m_colorFormat{};
    Image m_color;
    Image m_depth;
};

export class ViewportManager : VulkanResource
{
public:
    using VulkanResource::VulkanResource;

    ViewportId createViewport(ViewportCreateInfo&& info);
    void setViewportArea(ViewportId id, Rect area);
    [[nodiscard]] Rect getViewportArea(ViewportId id) const;
    [[nodiscard]] float getAspectRatio(ViewportId id) const;
    const Camera& getCamera(ViewportId id) const;
    void setCamera(ViewportId id, const Camera& camera);

    void update();
    void drawViewports(const RenderPassContext& renderContext);
    void recreateImages();
    void shutdown();

private:
    std::unordered_map<ViewportId, Viewport> m_viewports;
    ViewportId::ValueType m_nextId{};
};