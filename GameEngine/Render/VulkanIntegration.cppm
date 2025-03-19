module;
#define GLFW_INCLUDE_VULKAN
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

export module Engine:Render.Vulkan;
import vulkan_hpp;
import std;

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

export using ::GLFWwindow;
export using ::glfwGetFramebufferSize;
export using ::glfwInit;
export using ::glfwWindowHint;
export using ::glfwCreateWindow;
export using ::glfwSetWindowPos;
export using ::glfwSetWindowSize;
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
export using ::glfwGetWin32Window;
export using ::glfwMakeContextCurrent;
export using ::glfwSwapInterval;

export enum GetWindowLongOption
{
	GWL_Style = GWL_STYLE
};

export enum WindowStyle
{
	WS_Child = WS_CHILD
};

export namespace glfw
{
	constexpr auto ClientApi = GLFW_CLIENT_API;
	constexpr auto NoApi = GLFW_NO_API;
	constexpr auto Decorated = GLFW_DECORATED;
	constexpr auto Resizable = GLFW_RESIZABLE;
	constexpr auto True = GLFW_TRUE;
	constexpr auto False = GLFW_FALSE;

	vk::Result createWindowSurface(vk::Instance instance, GLFWwindow* window, const vk::AllocationCallbacks* allocator, vk::SurfaceKHR* surface)
	{
		return static_cast<vk::Result>(glfwCreateWindowSurface(instance, window, reinterpret_cast<const VkAllocationCallbacks*>(allocator), reinterpret_cast<VkSurfaceKHR*>(surface)));
	}
}

