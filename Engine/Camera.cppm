export module Engine.Camera;
import Math;

export struct Camera
{
    Mat4 view{};
    Mat4 proj{};
};

export namespace CameraUtils
{
    Vec3 right(const Camera&);
    Vec3 up(const Camera&);
    Vec3 forward(const Camera&);
    Vec3 position(const Camera&);
}