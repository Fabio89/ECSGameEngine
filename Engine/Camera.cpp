module Engine.Camera;

Vec3 CameraUtils::right(const Camera& camera)
{
    Mat4 inverseView = Math::inverse(camera.view);
    return Math::normalize(Vec3{inverseView[0]});
}

Vec3 CameraUtils::up(const Camera& camera)
{
    Mat4 inverseView = Math::inverse(camera.view);
    return Math::normalize(Vec3{inverseView[1]});
}

Vec3 CameraUtils::forward(const Camera& camera)
{
    Mat4 inverseView = Math::inverse(camera.view);
    return -Math::normalize(Vec3{inverseView[2]});
}

Vec3 CameraUtils::position(const Camera& camera)
{
    Mat4 inverseView = Math::inverse(camera.view);
    return Vec3{inverseView[3]};
}
