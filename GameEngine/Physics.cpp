module Physics;
import Component.BoundingBox;
import Component.Camera;
import Component.Name;
import Component.Transform;
import Core;

namespace Physics
{
    bool rayIntersectsAABB(const Ray& ray, const Vec3& aabbMin, const Vec3& aabbMax, float& tClosest);
    void checkNormalized(const Vec3& vector);
}

Entity Physics::lineTrace(const World& world, const Ray& ray, TraceChannelFlags channel)
{
    float closestHit = std::numeric_limits<float>::max();
    Entity hitEntity = invalidId();

    for (auto&& [entity, aabb] : world.view<BoundingBoxComponent>())
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
    log(closestHit < std::numeric_limits<float>::max() ? std::format("[Physics::lineTrace]: Hit '{}'", world.readComponent<NameComponent>(hitEntity).name) : "Miss");
    return hitEntity;
}

Ray Physics::rayFromScreenPosition(const World& world, const Player& player, Vec2 screenPosition)
{
    const Entity cameraEntity = player.getMainCamera();
    if (cameraEntity == invalidId() || !world.hasComponent<CameraComponent>(cameraEntity))
        return {};

    const CameraComponent& camera = world.readComponent<CameraComponent>(cameraEntity);
    const TransformComponent& cameraTransform = world.readComponent<TransformComponent>(cameraEntity);

    // Step 1: Get normalized device coordinates (NDC) of the cursor
    float x = screenPosition.x * 2.0f - 1.0f; // Convert [0..1] to [-1..1]
    float y = screenPosition.y * 2.0f - 1.0f; // Convert [0..1] to [-1..1]
    Vec4 rayClip = Vec4(x, y, -1.0f, 1.0f); // Camera clip space (near plane)

    // Step 2: Convert from clip space to view space
    Mat4 projInverse = Math::inverse(camera.projectionMatrix);
    Vec4 rayView = projInverse * rayClip;
    rayView.w = 0.0f; // Ensure direction vector
    Vec3 rayViewDirection = Vec3(rayView); // Normalize to create direction

    // Step 3: Convert from view space to world space
    Mat4 viewInverse = Math::inverse(camera.viewMatrix);
    Vec4 rayWorld = viewInverse * Vec4(rayViewDirection, 0.0f);
    Vec3 rayWorldDirection = Math::normalize(Vec3(rayWorld));

    // Step 4: Create the Ray (Origin is the camera position)
    Ray ray;
    ray.origin = cameraTransform.position; // Camera position in world space
    ray.direction = rayWorldDirection;

    // Output ray for debugging or raycasting logic
    //log(std::format("Ray: ({}, {}, {}) -> ({}, {}, {})", ray.origin.x, ray.origin.y, ray.origin.z, ray.direction.x, ray.direction.y, ray.direction.z));

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
