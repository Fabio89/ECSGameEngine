module;

#include <compare>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

export module Engine.Render.Core:Vulkan;
export import vulkan_hpp;

export namespace vk
{
	constexpr bool EnableValidationLayers =
#if NDEBUG
		false;
#else
		true;
#endif
}

export using ::GLFWwindow;
export using ::glfwGetFramebufferSize;
export using ::glfwInit;
export using ::glfwWindowHint;
export using ::glfwCreateWindow;
export using ::glfwSetWindowUserPointer;
export using ::glfwSetFramebufferSizeCallback;
export using ::glfwPollEvents;
export using ::glfwDestroyWindow;
export using ::glfwTerminate;
export using ::glfwWindowShouldClose;
export using ::glfwWaitEvents;
export using ::glfwGetRequiredInstanceExtensions;
export using ::glfwGetWindowUserPointer;
export using ::glfwCreateWindowSurface;

export namespace glfw
{
	constexpr auto ClientApi = GLFW_CLIENT_API;
	constexpr auto NoApi = GLFW_NO_API;
	constexpr auto Resizable = GLFW_RESIZABLE;
	constexpr auto True = GLFW_TRUE;

	vk::Result createWindowSurface(vk::Instance instance, GLFWwindow* window, const vk::AllocationCallbacks* allocator, vk::SurfaceKHR* surface)
	{
		return static_cast<vk::Result>(glfwCreateWindowSurface(static_cast<VkInstance>(instance), window, reinterpret_cast<const VkAllocationCallbacks*>(allocator), reinterpret_cast<VkSurfaceKHR*>(surface)));
	}
}

