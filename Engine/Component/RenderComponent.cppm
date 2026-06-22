export module Component.Render;
import Core;
import Math;
import Render.Model;
import Serialization;

export struct RenderComponent
{
    bool visible{true};
    MeshId mesh;
    TextureId texture;
    Mat4 model{1};
};

template <>
struct TypeTraits<RenderComponent>
{
    static constexpr auto name = "RenderComponent";
};