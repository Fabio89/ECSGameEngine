module Math;
import std;

// Quat Math::rotation(Vec3 origin, Vec3 destination)
// {
//     // Normalize both the origin and destination vectors
//     Vec3 normalizedOrigin = normalize(origin);
//     Vec3 normalizedDestination = normalize(destination);
//
//     // Calculate the axis of rotation (cross product)
//     Vec3 axis = cross(normalizedOrigin, normalizedDestination);
//
//     // If the cross product is nearly zero, the vectors are either the same or opposite
//     float angle = dot(normalizedOrigin, normalizedDestination);
//
//     // Clamp the dot product to avoid floating point errors resulting in NaNs for angle
//     angle = clamp(angle, -1.0f, 1.0f);
//
//     // Get the angle of rotation in radians
//     float theta = acos(angle);
//
//     // If the angle is very small, no rotation is needed, return identity quaternion
//     if (length(axis) < 0.001f) {
//         return Quat(1.f, 0.f, 0.f, 0.f); // Identity quaternion
//     }
//
//     // Normalize the axis of rotation
//     axis = normalize(axis);
//
//     // Calculate the quaternion
//     float halfTheta = theta * 0.5f;
//     float sinHalfTheta = sin(halfTheta);
//
//     return Quat(cos(halfTheta), -axis.x * sinHalfTheta, -axis.y * sinHalfTheta, -axis.z * sinHalfTheta);
// }
