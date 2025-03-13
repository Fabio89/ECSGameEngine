module;
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>

export module Engine:Math;
import std;

export using glm::qualifier;
export using glm::size_t;
export using glm::vec;
export using glm::vec2;
export using glm::vec3;
export using glm::ivec2;
export using glm::qua;
export using glm::mat;
export using glm::mat4;
export using glm::int8_t;
export using glm::int16_t;
export using glm::int32_t;
export using glm::int64_t;
export using glm::uint8_t;
export using glm::uint16_t;
export using glm::uint32_t;
export using glm::uint64_t;

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

export inline void hash_combine(size_t &seed, size_t hash)
{
	hash += 0x9e3779b9 + (seed << 6) + (seed >> 2);
	seed ^= hash;
}

export template<int N, typename T, qualifier Q>
size_t hash_value(const vec<N, T, Q>& v)
{
	return std::hash<vec<N, T, Q>>()(v);
}

