export module Component.LineRender;
import Core;
import Render.Model;

export struct LineRenderComponent
{
    Entity parent;
    std::vector<LineVertex> vertices;
};

template <>
struct TypeTraits<LineRenderComponent>
{
    static constexpr auto name = "LineRenderComponent";
};