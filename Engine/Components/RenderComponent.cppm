export module Component.Render;
import Core;
import Math;
import Properties;
import Render.Model;
import Serialization.Json;

export struct RenderComponent
{
    bool visible{true};
    MeshId mesh{invalidId()};
    TextureId texture{invalidId()};
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