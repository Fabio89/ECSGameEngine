export module Engine.Render.Core:QueueFamily;

import <vulkan/vulkan.hpp>;
import std;

export enum class QueueFamilyType : size_t
{
	Graphics,
	Present,
	Transfer,
	INDEX_TYPE_COUNT
};

export class QueueFamilyIndices
{
	std::vector<std::optional<uint32_t>> m_families;

public:
	QueueFamilyIndices();

	int size() const;

	const std::optional<uint32_t>& get(QueueFamilyType type) const;
	std::optional<uint32_t>& get(QueueFamilyType type);

	using Iterator = decltype(m_families)::iterator;
	Iterator begin() { return m_families.begin(); }
	Iterator end() { return m_families.end(); }

	using ConstIterator = decltype(m_families)::const_iterator;
	ConstIterator begin() const { return m_families.cbegin(); }
	ConstIterator end() const { return m_families.cend(); }
};

namespace QueueFamilyUtils
{
	export bool areAllIndicesSet(const QueueFamilyIndices&);
	export QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);
}

QueueFamilyIndices::QueueFamilyIndices()
	: m_families(static_cast<size_t>(QueueFamilyType::INDEX_TYPE_COUNT))
{}

int QueueFamilyIndices::size() const
{
	return static_cast<int>(m_families.size());
}

const std::optional<uint32_t>& QueueFamilyIndices::get(QueueFamilyType type) const
{
	return m_families.at(static_cast<size_t>(type));
}

std::optional<uint32_t>& QueueFamilyIndices::get(QueueFamilyType type)
{
	return m_families.at(static_cast<size_t>(type));
}

bool QueueFamilyUtils::areAllIndicesSet(const QueueFamilyIndices& indices)
{
	return std::ranges::all_of(indices, [](auto&& index) { return index.has_value(); });
}

QueueFamilyIndices QueueFamilyUtils::findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface)
{
	QueueFamilyIndices indices;

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

	for (uint32_t i = 0; i < queueFamilies.size(); ++i)
	{
		const VkQueueFamilyProperties& family = queueFamilies[i];
		if ((family.queueFlags & VK_QUEUE_GRAPHICS_BIT))
			indices.get(QueueFamilyType::Graphics) = i;
		else if ((family.queueFlags & VK_QUEUE_TRANSFER_BIT))
			indices.get(QueueFamilyType::Transfer) = i;

		{
			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
			if (presentSupport)
				indices.get(QueueFamilyType::Present) = i;
		}

		if (QueueFamilyUtils::areAllIndicesSet(indices))
			break;
	}

	auto isGraphicsFamily = [](const VkQueueFamilyProperties& family) { return (family.queueFlags & VK_QUEUE_GRAPHICS_BIT); };
	if (auto it = std::ranges::find_if(queueFamilies, isGraphicsFamily); it != queueFamilies.end())
	{
		indices.get(QueueFamilyType::Graphics) = static_cast<uint32_t>(it - queueFamilies.begin());
	}

	return indices;
}