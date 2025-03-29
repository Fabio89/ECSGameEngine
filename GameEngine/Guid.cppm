module;
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

export module Guid;
import Core;

using namespace boost::uuids;

random_generator& getRandomGenerator()
{
    static std::mutex uuidGeneratorMutex;
    static random_generator generator;
    std::lock_guard lock{uuidGeneratorMutex};
    return generator;
}

export class Guid
{
public:
    Guid() = default;

    static Guid createFromString(const std::string& str) { return Guid{string_generator()(str)}; }
    static Guid createRandom() { return Guid{getRandomGenerator()()}; }

    explicit operator bool() const { return !m_impl.is_nil(); }
    
    bool operator==(const Guid& other) const { return m_impl == other.m_impl; }

    auto operator<=>(const Guid& other) const
    {
        if (m_impl == other.m_impl)
            return std::strong_ordering::equal;
        if (m_impl < other.m_impl)
            return std::strong_ordering::less;
        return std::strong_ordering::greater;
    }

    std::string toString() const { return to_string(m_impl); }

    friend std::ostream& operator<<(std::ostream& os, const Guid& guid)
    {
        os << guid.m_impl;
        return os;
    }

    friend std::istream& operator>>(std::istream& is, Guid& guid)
    {
        is >> guid.m_impl;
        return is;
    }

    std::size_t hashValue() const { return hash_value(m_impl); }

private:
    explicit Guid(uuid&& uuid) : m_impl{std::move(uuid)}
    {
    }

    uuid m_impl{};
};

template <>
struct std::hash<Guid>
{
    std::size_t operator ()(const Guid& value) const noexcept
    {
        return value.hashValue();
    }
};
