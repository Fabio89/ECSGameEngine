export module Components.LineRender;
import Assets.Mesh;
import Core;

export struct LineRenderComponent
{
    std::vector<LineVertex> vertices{};
};

template<>
constexpr std::string_view getTypeName<LineRenderComponent>() { return "LineRenderComponent"; }
