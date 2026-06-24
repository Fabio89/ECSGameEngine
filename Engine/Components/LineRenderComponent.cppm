export module Component.LineRender;
import Core;
import Render.Model;

export struct LineRenderComponent
{
    std::vector<LineVertex> vertices{};
};

template<>
constexpr std::string_view getTypeName<LineRenderComponent>() { return "LineRenderComponent"; }
