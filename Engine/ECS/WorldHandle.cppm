export module WorldHandle;
import Core;

export struct WorldHandle
{
    UInt32 index{invalidIndex};
    UInt32 generation{};

    static constexpr UInt32 invalidIndex = std::numeric_limits<UInt32>::max();

    [[nodiscard]] bool isValid() const { return index != invalidIndex; }

    friend bool operator==(const WorldHandle&, const WorldHandle&) = default;
};

template<>
struct std::hash<WorldHandle>
{
    size_t operator()(const WorldHandle& handle) const noexcept
    {
        size_t seed = std::hash<UInt32>{}(handle.index);

        seed ^= std::hash<UInt32>{}(handle.generation)
                + 0x9e3779b9
                + (seed << 6)
                + (seed >> 2);

        return seed;
    }
};