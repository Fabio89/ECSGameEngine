export module Component.Render;
import Assets.Mesh;
import Assets.Texture;
import Core;
import Math;
import Properties;
import Serialization.Json;

export struct RenderComponent
{
    bool visible{true};
    std::size_t mesh{};
    std::size_t texture{};
    Mat4 model{1};
};

template<>
constexpr std::string_view getTypeName<RenderComponent>() { return "RenderComponent"; }

template<>
struct TypeProperties<RenderComponent>
{
    static constexpr std::tuple list{
        makeProperty("visible", &RenderComponent::visible),
        makeProperty("mesh", &RenderComponent::mesh),
        makeProperty("texture", &RenderComponent::texture),
        makeProperty("model", &RenderComponent::model)
    };
};