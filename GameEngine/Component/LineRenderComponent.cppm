export module Component.LineRender;
import Core;
import Render.Model;

export struct LineRenderComponent : Component<LineRenderComponent>
{
    Entity parent;
    std::vector<LineVertex> vertices;
};

template <>
struct TypeTraits<LineRenderComponent>
{
    static constexpr auto name = "LineRenderComponent";
};