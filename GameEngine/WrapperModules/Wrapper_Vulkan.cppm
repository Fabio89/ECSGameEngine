export module Wrapper.Vulkan;
export import vulkan_hpp;

export constexpr size_t MaxFramesInFlight{2};

export namespace vk
{
    constexpr bool EnableValidationLayers =
#if NDEBUG
        false;
#else
            true;
#endif
}