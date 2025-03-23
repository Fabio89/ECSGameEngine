export module Math;
export import Core;
import <glm/gtc/quaternion.hpp>;

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
	using glm::min;
	using glm::max;
	using glm::radians;
	using glm::lookAt;
	using glm::perspective;
	using glm::rotate;
	using glm::translate;
	using glm::scale;
	using glm::normalize;
	using glm::eulerAngles;
	using glm::epsilonEqual;
	using glm::epsilon;
	using glm::distance;
	using glm::mat4_cast;
	using glm::cross;
	using glm::dot;
	
	Quat rotation(Vec3 origin, Vec3 destination);
}

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

Quat Math::rotation(Vec3 origin, Vec3 destination)
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
