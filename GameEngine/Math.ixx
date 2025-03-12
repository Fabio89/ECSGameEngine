module;
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>
export module Math;

export using glm::size_t;
export using glm::vec;
export using glm::vec2;
export using glm::vec3;
export using glm::ivec2;
export using glm::qua;
export using glm::tdualquat;
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

export template <int N, typename T>
size_t hash_vector(vec<N, T> const& v) { return std::hash<vec<N, T>>{}(v); }

// export template <typename T, glm::qualifier Q>
// inline std::size_t hash_value(vec<3, T, Q>)
// {
//     return std::hash<vec<1, T, Q>>()();
// }

// export struct std::hash<vec3>;
//
// export template <typename T, glm::qualifier Q>
// struct std::hash<vec<1, T, Q>>;
//
// export template <typename T, glm::qualifier Q>
// struct std::hash<vec<2, T, Q>>;
//
// export template <typename T, glm::qualifier Q>
// struct std::hash<vec<3, T, Q>>;
//
// export template <typename T, glm::qualifier Q>
// struct std::hash<vec<4, T, Q>>;
//
// export template <typename T, glm::qualifier Q>
// struct std::hash<qua<T, Q>>;
//
// export template <typename T, glm::qualifier Q>
// struct std::hash<tdualquat<T, Q>>;
//
// export template <typename T, glm::qualifier Q>
// struct std::hash<mat<2, 2, T, Q>>;
//
// export template <typename T, glm::qualifier Q>
// struct std::hash<mat<2, 3, T, Q>>;
//
// export template <typename T, glm::qualifier Q>
// struct std::hash<mat<2, 4, T, Q>>;
//
// export template <typename T, glm::qualifier Q>
// struct std::hash<mat<3, 2, T, Q>>;
//
// export template <typename T, glm::qualifier Q>
// struct std::hash<mat<3, 3, T, Q>>;
//
// export template <typename T, glm::qualifier Q>
// struct std::hash<mat<3, 4, T, Q>>;
//
// export template <typename T, glm::qualifier Q>
// struct std::hash<mat<4, 2, T, Q>>;
//
// export template <typename T, glm::qualifier Q>
// struct std::hash<mat<4, 3, T, Q>>;
//
// export template <typename T, glm::qualifier Q>
// struct std::hash<mat<4, 4, T, Q>>;
