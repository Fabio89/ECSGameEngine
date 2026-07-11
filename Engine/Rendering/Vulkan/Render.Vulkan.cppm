export module Render.Vulkan;
export import vulkan;
export import std;

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