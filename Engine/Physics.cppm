module;

#include "EngineExport.h"

export module Physics;
import Engine.Camera;
import Math;
import World;

export
enum class TraceChannelFlags : UInt64
{
    None = 0,
    Default = 1 << 0,
    WorldStatic = 1 << 1,
    WorldDynamic = 1 << 2,
    Gizmo = 1 << 3,
    UI = 1 << 4,
};

export
using TraceChannelFlagsType = std::underlying_type_t<TraceChannelFlags>;

export
inline TraceChannelFlags operator|(TraceChannelFlags a, TraceChannelFlags b)
{
    return static_cast<TraceChannelFlags>(static_cast<TraceChannelFlagsType>(a) | static_cast<TraceChannelFlagsType>(b));
}

inline TraceChannelFlags operator&(TraceChannelFlags a, TraceChannelFlags b)
{
    return static_cast<TraceChannelFlags>(static_cast<TraceChannelFlagsType>(a) & static_cast<TraceChannelFlagsType>(b));
}

export
class TraceChannel
{
public:
    TraceChannel() = default;
    explicit TraceChannel(TraceChannelFlags channelFlags) : m_mask{static_cast<TraceChannelFlagsType>(channelFlags)} {}

    void set(TraceChannelFlags channelFlags, bool value = true);
    void reset(TraceChannelFlags channelFlags);

    [[nodiscard]] bool test(TraceChannelFlags flags) const { return (m_mask & flags) == flags; }
    [[nodiscard]] TraceChannelFlagsType toNumber() const { return static_cast<TraceChannelFlagsType>(m_mask); }

private:
    TraceChannelFlags m_mask{};
};

export
struct Ray
{
    Vec3 origin{};
    Vec3 direction{};
};

namespace Physics
{
    export ENGINE_API
    Entity lineTrace(const World& world, const Ray& ray, TraceChannelFlags channel);

    export ENGINE_API
    Ray rayFromViewportUV(const Camera& camera, Vec2 uv);

    export
    std::optional<Vec3> intersectRayPlane(const Ray& ray, const Plane& plane);
}
