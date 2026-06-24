export module Component.LineRender;
import Core;
import Render.Model;

export struct LineRenderComponent
{
    std::vector<LineVertex> vertices{};
};

template<>
struct TypeTraits<LineRenderComponent>
{
    static constexpr std::string_view name = "LineRenderComponent";
};
