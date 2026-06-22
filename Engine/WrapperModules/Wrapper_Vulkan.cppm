export module Wrapper.Vulkan;
export import vulkan_hpp;
import std;

export constexpr std::size_t MaxFramesInFlight{2};

export namespace vk
{
    constexpr bool EnableValidationLayers =
#if NDEBUG
        false;
#else
            true;
#endif
}