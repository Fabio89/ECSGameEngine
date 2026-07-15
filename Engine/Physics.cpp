module Physics;
import Components.BoundingBox;
import Components.Camera;
import Components.Name;
import Components.Transform;
import Core;

namespace Physics
{
    bool rayIntersectsAABB(const Ray& ray, const Vec3& aabbMin, const Vec3& aabbMax, float& tClosest);
    void checkNormalized(const Vec3& vector);
}

Entity Physics::lineTrace(const World& world, const Ray& ray, TraceChannelFlags channel)
{
    float closestHit = std::numeric_limits<float>::max();
    Entity hitEntity;

    for (auto&& [entity, aabb] : world.query<BoundingBoxComponent>())
    {
        if (!aabb.channel.test(channel))
            continue;
        
        float tClosest;
        if (rayIntersectsAABB(ray, aabb.minWorld, aabb.maxWorld, tClosest))
        {
            if (tClosest < closestHit)
            {
                closestHit = tClosest;
                hitEntity = entity;
            }
        }
    }
    log(closestHit < std::numeric_limits<float>::max() ? std::format("[Physics::lineTrace]: Hit '{}'", NameUtils::getName(world, hitEntity)) : "Miss");
    return hitEntity;
}

Ray Physics::rayFromViewportUV(const Camera& camera, Vec2 uv)
{
    const float x = uv.x * 2.0f - 1.0f;
    const float y = uv.y * 2.0f - 1.0f;

    const Vec4 rayClip{x, y, 1.f, 1.f};

    const Mat4 invProj = Math::inverse(camera.proj);
    Vec4 rayView = invProj * rayClip;
    rayView = Vec4(rayView.x, rayView.y, 1.f, 0.f);

    const Mat4 invView = Math::inverse(camera.view);

    Ray ray;
    ray.origin = Vec3{invView * Vec4{0, 0, 0, 1}};
    ray.direction = Math::normalize(Vec3{invView * rayView});

    return ray;
}

std::optional<Vec3> Physics::intersectRayPlane(const Ray& ray, const Plane& plane)
{
    checkNormalized(ray.direction);
    checkNormalized(plane.normal);

    const float denominator = Math::dot(plane.normal, ray.direction);
    if (Math::abs(denominator) < Math::epsilon<float>()) // Parallel, no intersection
        return std::nullopt;

    const float t = Math::dot(plane.point - ray.origin, plane.normal) / denominator;
    if (t < 0.0f)
        return std::nullopt; // Intersection behind the ray origin

    return ray.origin + ray.direction * t;
}

bool Physics::rayIntersectsAABB(const Ray& ray, const Vec3& aabbMin, const Vec3& aabbMax, float& tClosest)
{
    float tMin = 0.0f;
    float tMax = std::numeric_limits<float>::max();

    for (int i = 0; i < 3; ++i)
    {
        // Check ray's direction against the bounding box planes
        if (std::abs(ray.direction[i]) > std::numeric_limits<float>::epsilon())
        {
            float t1 = (aabbMin[i] - ray.origin[i]) / ray.direction[i];
            float t2 = (aabbMax[i] - ray.origin[i]) / ray.direction[i];

            if (t1 > t2) std::swap(t1, t2);

            tMin = Math::max(tMin, t1);
            tMax = Math::min(tMax, t2);

            if (tMax < tMin)
                return false;
        }
        else if (ray.origin[i] < aabbMin[i] || ray.origin[i] > aabbMax[i])
        {
            return false; // Ray is parallel and outside the slab
        }
    }

    tClosest = tMin;
    return true;
}

void Physics::checkNormalized(const Vec3& vector)
{
}

void TraceChannel::set(TraceChannelFlags channelFlags, bool value)
{
    using FlagsType = std::underlying_type_t<TraceChannelFlags>;
    FlagsType result;
    if (value)
        result = static_cast<FlagsType>(m_mask) | static_cast<FlagsType>(channelFlags);
    else
        result = static_cast<FlagsType>(m_mask) & ~static_cast<FlagsType>(channelFlags);
    m_mask = static_cast<TraceChannelFlags>(result);
}

void TraceChannel::reset(TraceChannelFlags channelFlags)
{
    using FlagsType = std::underlying_type_t<TraceChannelFlags>;
    const auto result = static_cast<FlagsType>(m_mask) & ~static_cast<FlagsType>(channelFlags);
    m_mask = static_cast<TraceChannelFlags>(result);
}
