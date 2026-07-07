export module WorldHandle;
import Core;

export struct WorldHandle
{
    UInt32 index{invalidIndex};
    UInt32 generation{};

    static constexpr UInt32 invalidIndex = std::numeric_limits<UInt32>::max();

    bool isValid() const { return index != invalidIndex; }

    friend bool operator==(const WorldHandle&, const WorldHandle&) = default;
};