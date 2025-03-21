module;
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

export module Wrapper.BoostUuid;

// ReSharper disable CppClangTidyMiscUnusedUsingDecls
export using boost::uuids::string_generator;
export using boost::uuids::random_generator;
export using boost::uuids::uuid;
export using boost::uuids::hash_value;
export using boost::uuids::operator==;
export using boost::uuids::operator!=;
export using boost::uuids::operator<;
export using boost::uuids::operator>;
export using boost::uuids::operator<=;
export using boost::uuids::operator>=;
export using boost::uuids::operator<<;
export using boost::uuids::operator>>;
export using boost::uuids::swap;
export using boost::uuids::to_string;