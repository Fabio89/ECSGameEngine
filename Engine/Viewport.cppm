export module Engine.Viewport;
import Core;

export struct ViewportId : Id<struct ViewportTag> {};

template<>
struct std::hash<ViewportId>
{
    constexpr std::size_t operator()(const ViewportId& id) const noexcept
    {
        return std::hash<Id<ViewportTag>>{}(id);
    }
};