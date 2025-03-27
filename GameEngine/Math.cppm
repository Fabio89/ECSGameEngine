module;
// ReSharper disable CppWrongIncludesOrder
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_LEFT_HANDED
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
// ReSharper disable once CppUnusedIncludeDirective
#include <glm/ext/quaternion_double_precision.hpp> // won't compile without this
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/dual_quaternion.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/transform.hpp>
// ReSharper restore CppWrongIncludesOrder

export module Math;
export import Core;
import Log;

using glm::qualifier;
using glm::vec;
using glm::qua;
using glm::mat;

export using Vec2 = glm::vec2;
export using Vec3 = glm::vec3;
export using Vec4 = glm::vec4;
export using IVec2 = glm::ivec2;
export using Quat = glm::quat;
export using Mat4 = glm::mat4;

export using glm::operator==;
export using glm::operator!=;
export using glm::operator*;
export using glm::operator/;
export using glm::operator+;
export using glm::operator-;

export namespace Math
{
    using glm::all;
    using glm::equal;
    using glm::min;
    using glm::max;
    using glm::clamp;
    using glm::length;
    using glm::radians;
    using glm::degrees;
    using glm::angleAxis;
    using glm::pitch;
    using glm::yaw;
    using glm::roll;
    using glm::lookAt;
    using glm::perspective;
    using glm::translate;
    using glm::rotate;
    using glm::scale;
    using glm::normalize;
    using glm::eulerAngles;
    using glm::epsilonEqual;
    using glm::epsilon;
    using glm::distance;
    using glm::mat4_cast;
    using glm::sin;
    using glm::cos;
    using glm::tan;
    using glm::asin;
    using glm::acos;
    using glm::atan;
    using glm::cross;
    using glm::dot;
    using glm::inverse;

    using glm::rotation;

    using glm::lerp;
    using glm::slerp;
    using glm::smoothstep;
    // template<typename T, qualifier Q>
    // GLM_FUNC_QUALIFIER T pitch(qua<T, Q> const& q)
    // {
    //     log("Using pitch");
    //     // Corrected Pitch (rotation around X-axis)
    //     T sinp = 2 * (q.w * q.x - q.y * q.z);
    //
    //     // Small numerical inaccuracies can cause near-zero deviations, force these to zero:
    //     if (std::abs(sinp) < static_cast<T>(1e-6)) {
    //         return static_cast<T>(0); // Explicitly treat no X-axis rotation as zero
    //     }
    //
    //     // Otherwise, safely compute pitch using arcsine:
    //     return static_cast<T>(asin(clamp(sinp, static_cast<T>(-1), static_cast<T>(1))));
    // }
    //
    //
    //
    //
    // template<typename T, qualifier Q>
    // GLM_FUNC_QUALIFIER T yaw(qua<T, Q> const& q)
    // { log("Using yaw");
    //     // Yaw (rotation around Y-axis)
    //     return static_cast<T>(atan2(2 * (q.w * q.y + q.z * q.x), 1 - 2 * (q.y * q.y + q.x * q.x)));
    // }
    //
    // template<typename T, qualifier Q>
    // GLM_FUNC_QUALIFIER T roll(qua<T, Q> const& q)
    // {log("Using roll");
    //     return static_cast<T>(atan2(2 * (q.w * q.z + q.x * q.y), 1 - 2 * (q.z * q.z + q.x * q.x)));
    // }






}

export constexpr Vec3 forwardVector() { return { 0.0f, 0.0f, 1.0f }; }
export constexpr Vec3 rightVector() { return {1.0f, 0.0f, 0.0f}; }
export constexpr Vec3 upVector() { return {0.0f, 1.0f, 0.0f}; }

export inline void hash_combine(size_t& seed, size_t hash)
{
    hash += 0x9e3779b9 + (seed << 6) + (seed >> 2);
    seed ^= hash;
}

export template <typename T, qualifier Q>
size_t hash_value(const vec<2, T, Q>& v)
{
    auto hasher = std::hash<T>{};
    auto seed = hasher(v.x);
    hash_combine(seed, hasher(v.y));
    return seed;
}

export template <typename T, qualifier Q>
size_t hash_value(const vec<3, T, Q>& v)
{
    auto hasher = std::hash<T>{};
    auto seed = hasher(v.x);
    hash_combine(seed, hasher(v.y));
    hash_combine(seed, hasher(v.z));
    return seed;
}
