export module Math;
import std;
import <glm/gtc/quaternion.hpp>;

using glm::qualifier;
using glm::vec;
using glm::qua;
using glm::mat;

export using Vec2 = glm::vec2;
export using Vec3 = glm::vec3;
export using IVec2 = glm::ivec2;
export using Quat = glm::quat;
export using Mat4 = glm::mat4;
export using Int8 = std::int8_t;
export using Int16 = std::int16_t;
export using Int32 = std::int32_t;
export using Int64 = std::int64_t;
export using UInt8 = std::uint8_t;
export using UInt16 = std::uint16_t;
export using UInt32 = std::uint32_t;
export using UInt64 = std::uint64_t;

export using glm::operator==;
export using glm::operator!=;
export using glm::operator*;
export using glm::operator/;
export using glm::operator+;
export using glm::operator-;

export using glm::radians;
export using glm::lookAt;
export using glm::perspective;
export using glm::rotate;
export using glm::translate;
export using glm::scale;
export using glm::normalize;
export using glm::eulerAngles;
export using glm::epsilonEqual;
export using glm::epsilon;
export using glm::distance;
export using glm::mat4_cast;
export using glm::cross;
export using glm::dot;

export constexpr Vec3 forwardVector() { return {1.0f, 0.0f, 0.0f}; }
export constexpr Vec3 rightVector() { return {0.0f, 1.0f, 0.0f}; }
export constexpr Vec3 upVector() { return {0.0f, 0.0f, 1.0f}; }

export inline void hash_combine(size_t &seed, size_t hash)
{
	hash += 0x9e3779b9 + (seed << 6) + (seed >> 2);
	seed ^= hash;
}

export template<typename T, qualifier Q>
size_t hash_value(const vec<2, T, Q>& v)
{
	auto hasher = std::hash<T>{};
	auto seed = hasher(v.x);
	hash_combine(seed, hasher(v.y));
	return seed;
}

export template<typename T, qualifier Q>
size_t hash_value(const vec<3, T, Q>& v)
{
	auto hasher = std::hash<T>{};
	auto seed = hasher(v.x);
	hash_combine(seed, hasher(v.y));
	hash_combine(seed, hasher(v.z));
	return seed;
}

export Quat rotation(Vec3 origin, Vec3 destination)
{
	// Normalize both the origin and destination vectors
	Vec3 normalizedOrigin = normalize(origin);
	Vec3 normalizedDestination = normalize(destination);

	// Calculate the axis of rotation (cross product)
	Vec3 axis = cross(normalizedOrigin, normalizedDestination);

	// If the cross product is nearly zero, the vectors are either the same or opposite
	float angle = dot(normalizedOrigin, normalizedDestination);

	// Clamp the dot product to avoid floating point errors resulting in NaNs for angle
	angle = glm::clamp(angle, -1.0f, 1.0f);

	// Get the angle of rotation in radians
	float theta = glm::acos(angle);

	// If the angle is very small, no rotation is needed, return identity quaternion
	if (glm::length(axis) < 0.001f) {
		return Quat(1.f, 0.f, 0.f, 0.f); // Identity quaternion
	}

	// Normalize the axis of rotation
	axis = normalize(axis);

	// Calculate the quaternion
	float halfTheta = theta * 0.5f;
	float sinHalfTheta = glm::sin(halfTheta);

	return Quat(glm::cos(halfTheta), -axis.x * sinHalfTheta, -axis.y * sinHalfTheta, -axis.z * sinHalfTheta);
}
